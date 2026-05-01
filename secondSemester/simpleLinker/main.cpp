#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include <algorithm>
#include <optional>
#include <memory>
#include <stdexcept>
#include <set>
#include <map>
#include <queue>
#include <deque>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#endif /* __PROGTEST__ */

using sint = uint32_t;

enum class State
{
    Fresh,
    Open,
    Closed
};

/**
 * Class used for representing a function node with its data and dependencies
 */
class LinkerFunc
{
    /**
     * Class that represents LinkerFunc's dependency on another LinkerFunc
     */
    struct Dependency
    {
        /** @var pointer to a dependency function */
        std::weak_ptr<LinkerFunc> func;
        /** @var reference position (within the function code) */
        sint locationRelativeToStart;

    public:
        Dependency(const std::shared_ptr<LinkerFunc>& func, const sint loc) : func(func), locationRelativeToStart(loc)
        {
        }
    };

    /** @var function name */
    std::string name;
    /** @var function's binary data */
    std::string data;

public:
    /** @var decides whether the function has already been included during linking process*/
    State state;
    /** @var pointer to data location within the imported file*/
    /** @var list of all dependencies*/
    std::vector<Dependency> imports;

    /**Hashing function for shared_ptr to LinkerFunc*/
    struct LinkerFuncHasher
    {
        std::size_t operator()(const std::shared_ptr<LinkerFunc>& ptr) const
        {
            return std::hash<std::string>{}(ptr->name);
        }
    };

    /**compare functor for unordered_set*/
    struct LinkerFuncEqual
    {
        bool operator()(const std::shared_ptr<LinkerFunc>& a,
                        const std::shared_ptr<LinkerFunc>& b) const
        {
            return a->name == b->name;
        }
    };


    LinkerFunc(std::string name, std::string data) : name(std::move(name)), data(std::move(data)), state(State::Fresh)
    {
    }

    explicit LinkerFunc(std::string name) : name(std::move(name)), data(), state(State::Fresh)
    {
    }

    std::string& getName()
    {
        return name;
    }

    std::string& getData()
    {
        return data;
    }

    void setData(std::string str)
    {
        std::swap(data, str);
    }

    void pushImport(const Dependency& dep)
    {
        imports.push_back(dep);
    }

    //todo changed from codeLoc to name, watch out for using this in containers
    bool operator<(const LinkerFunc& func) const
    {
        return name < func.name;
    }
};

/** Function linker. Used for adding values and then linking them to a file*/
class CLinker
{
    using LinkerPtr = std::shared_ptr<LinkerFunc>;
    /** @var hashset with its own comp func (because they're smart ptrs)*/
    std::unordered_set<LinkerPtr, LinkerFunc::LinkerFuncHasher, LinkerFunc::LinkerFuncEqual> funcs;

public:
    CLinker() = default;
    CLinker(const CLinker& c) = delete;

    ~CLinker() = default;

    CLinker& operator=(CLinker linker) = delete;

    /**
     * Adds functions implementation and their exports to the linker
     * @param fileName name of file with binary data to add to linker
     * @returns reference to itself
     * @throws std::runtime_error when the file is not in correct format
     */
    CLinker& addFile(const std::string& fileName)
    {
        //will contain only exported functions
        //used when assigning import funcs and reading data (because it's sorted by code start location)
        std::map<sint, LinkerPtr> currentExpFuncs = {};
        std::ifstream file(fileName, std::ios::binary);

        if (!file || !file.is_open())
        {
            throw std::runtime_error("File not available");
        }

        //reading 0-b bytes
        const sint exportsCount = readVar<sint>(file);
        sint importsCount = readVar<sint>(file);
        sint codeLengthBytes = readVar<sint>(file);

        if (!file)
        {
            throw std::runtime_error("Wrong file format");
        }

        if (exportsCount == 0)
        {
            throw std::runtime_error("No export functions");
        }

        //exporting funcs
        for (sint i = 0; i < exportsCount; ++i)
        {
            const char nameLen = readVar<char>(file);

            std::string name;
            name.resize(nameLen);
            file.read(&name[0], nameLen);

            const sint funcAddr = readVar<sint>(file);
            LinkerPtr ptr = std::make_shared<LinkerFunc>(std::move(name));

            auto ret = funcs.insert(ptr);
            //if already exists
            if (!ret.second)
            {
                //could be created from imports, so gotta check if it has data
                //if it's twice in the same file it doesn't have data yet, but it would be included in currentExpFuncs
                if (!(*ret.first)->getData().empty() || funcNameContained(currentExpFuncs, (*ret.first)->getName()))
                {
                    throw std::runtime_error("Function exported twice");
                }
                currentExpFuncs[funcAddr] = *ret.first;
                continue;
            }
            currentExpFuncs[funcAddr] = ptr;
        }

        if (!file)
        {
            throw std::runtime_error("Wrong file format");
        }

        //importing funcs
        for (sint i = 0; i < importsCount; ++i)
        {
            const char nameLen = readVar<char>(file);

            std::string name;
            name.resize(nameLen);
            file.read(&name[0], nameLen);

            LinkerPtr toFindPtr = std::make_shared<LinkerFunc>(std::move(name));

            //insert it, if I get a non-existing func, there will be a func with no data that equals runtime_error during linking
            //don't need to insert it into the ordered_set, that is just for efficiently reading data
            const auto insertAttempt = funcs.insert(toFindPtr);

            //LinkerFunc debug = *insertAttempt.first.operator*();

            const sint refCount = readVar<sint>(file);

            //go through all the references
            for (sint j = 0; j < refCount; ++j)
            {
                sint codeRef = readVar<sint>(file);

                //find the function that has that block of code
                //(the highest starting func that is lower than codeRef(starts before codeRef))
                auto dependentFunc = currentExpFuncs.upper_bound(codeRef);

                if (dependentFunc == currentExpFuncs.begin())
                {
                    throw std::runtime_error("Bad import func reference: no function starts before codeRef");
                }
                --dependentFunc;

                //add dependency, with pointer to this imported func, location relative to the function's data start
                dependentFunc->second->imports.emplace_back(*insertAttempt.first, codeRef - dependentFunc->first);
            }
        }

        if (!file)
        {
            throw std::runtime_error("Wrong file format");
        }

        //loading functions code
        //two pointers to find the function's code length
        auto firstPtr = currentExpFuncs.begin();
        auto secondPtr = ++currentExpFuncs.begin();

        while (secondPtr != currentExpFuncs.end())
        {
            sint dataLen = secondPtr->first - firstPtr->first;
            std::string data;
            data.resize(dataLen);
            file.read(&data[0], dataLen);

            firstPtr->second->setData(std::move(data));

            if (!file)
            {
                throw std::runtime_error("Wrong file format");
            }

            ++firstPtr;
            ++secondPtr;
        }
        sint dataLen = codeLengthBytes - firstPtr->first;


        std::string data;
        data.resize(dataLen);
        file.read(&data[0], dataLen);

        firstPtr->second->setData(std::move(data));

        return *this;
    }

    /**
     * Writes linked code in binary to the specified filename
     * @param fileName output file name
     * @param entryPoint name of the main function
     */
    void linkOutput(const std::string& fileName, const std::string& entryPoint)
    {
        std::ofstream file(fileName, std::ios::binary);

        if (!file || !file.is_open())
        {
            throw std::runtime_error("File not available");
        }

        //set all funcs to Fresh state
        for (auto& node : funcs)
        {
            node->state = State::Fresh;
        }

        //used for storing the location of code for each function (when it gets written in the file)
        std::map<LinkerPtr, sint> funcLocs;

        LinkerPtr toFindMain = std::make_shared<LinkerFunc>(entryPoint);
        auto it = funcs.find(toFindMain);
        if (it == funcs.end())
        {
            throw std::runtime_error("Starting function not found");
        }

        //recursive call chain through the entire graph
        includeFunc(file, funcLocs, *it);
    }

private:
    sint includeFunc(std::ofstream& file, std::map<LinkerPtr, sint>& funcLocs,
                     const LinkerPtr& func)
    {
        //if state is not Fresh, it was already written into the file -> return the code's location
        if (func->state != State::Fresh)
        {
            return funcLocs[func];
        }
        func->state = State::Open;


        file.seekp(0, std::ios::end);
        sint currentFuncLocation = file.tellp();
        funcLocs.emplace(func, currentFuncLocation);

        if (func->getData().empty())
        {
            throw std::runtime_error("Import was never exported");
        }

        file.write(func->getData().c_str(), func->getData().size());

        if (!file)
        {
            throw std::runtime_error("Error while writing into selected file");
        }

        //go through all the function's references
        for (auto& weakDep : func->imports)
        {
            LinkerPtr dep = weakDep.func.lock();

            //get referenced function's code location with recursive call
            const sint refLoc = includeFunc(file, funcLocs, dep);

            //set to the referenced 4 bytes and overwrite them with the location
            file.seekp(currentFuncLocation + weakDep.locationRelativeToStart, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&refLoc), sizeof(sint));
        }

        func->state = State::Closed;
        return currentFuncLocation;
    }

    template <typename T>
    T readVar(std::ifstream& file)
    {
        T var{};
        file.read(reinterpret_cast<char*>(&var), sizeof(T));
        // readVar(file, sizeof(T));

        return var;
    }

    static bool funcNameContained(const std::map<sint, LinkerPtr>& currentExpFuncs, const std::string& name)
    {
        for (auto& pair : currentExpFuncs)
        {
            if (pair.second->getName() == name)
            {
                return true;
            }
        }
        return false;
    }
};

#ifndef __PROGTEST__

int main()
{
    CLinker().addFile("0in0.o").linkOutput("0out", "main");

    CLinker().addFile("0in0.o").linkOutput("0out", "strlen");

    CLinker().addFile("1in0.o").linkOutput("1out", "main");

    CLinker c{};
    c.addFile("2in0.o").addFile("2in1.o").linkOutput("2out", "main");

    CLinker().addFile("3in0.o").addFile("3in1.o").linkOutput("3out", "towersOfHanoi");

    try
    {
        CLinker().addFile("4in0.o").addFile("4in1.o").linkOutput("4out", "unusedFunc");
        assert("missing an exception" == nullptr);
    }
    catch (const std::runtime_error& e)
    {
        // e . what (): Undefined symbol qsort
    }
    catch (...)
    {
        assert("invalid exception" == nullptr);
    }

    try
    {
        CLinker().addFile("5in0.o").linkOutput("5out", "main");
        assert("missing an exception" == nullptr);
    }
    catch (const std::runtime_error& e)
    {
        // e . what (): Duplicate symbol: printf
    }
    catch (...)
    {
        assert("invalid exception" == nullptr);
    }

    try
    {
        CLinker().addFile("6in0.o").linkOutput("6out", "strlen");
        assert("missing an exception" == nullptr);
    }
    catch (const std::runtime_error& e)
    {
        // e . what (): Cannot read input file
    }
    catch (...)
    {
        assert("invalid exception" == nullptr);
    }

    try
    {
        CLinker().addFile("7in0.o").linkOutput("7out", "strlen2");
        assert("missing an exception" == nullptr);
    }
    catch (const std::runtime_error& e)
    {
        // e . what (): Undefined symbol strlen2
    }
    catch (...)
    {
        assert("invalid exception" == nullptr);
    }

    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
