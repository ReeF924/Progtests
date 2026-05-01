#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#endif /* __PROGTEST__ */

class OwnString
{
    friend void asserts();
    char* data;
    size_t length = 0; //length is counted without the \0
    size_t capacity = 0;
    size_t refCount = 1;

public:
    OwnString(const char* c = "") : data(nullptr)
    {
        append(c, strlen(c));
    }

    OwnString(const OwnString& toCopy) : data(new char [toCopy.length + 1]), length(toCopy.length),
                                         capacity(toCopy.length + 1)
    {
        strncpy(data, toCopy.data, length + 1);
    }

    OwnString(OwnString&& toMove) noexcept : data(toMove.data), length(toMove.length), capacity(toMove.capacity)
    {
        toMove.data = nullptr;
        toMove.capacity = 0;
        toMove.length = 0;
    }

    ~OwnString() noexcept
    {
        delete [] data;
    }

    OwnString& operator=(OwnString right) noexcept
    {
        std::swap(data, right.data);
        std::swap(right.length, length);
        std::swap(right.capacity, capacity);

        return *this;
    }

    bool operator==(const OwnString& a) const
    {
        return strcmp(data, a.data);
    }

    size_t size() const
    {
        return length;
    }

    friend std::ostream& operator <<(std::ostream& ostream, const OwnString& string)
    {
        //could be a problem if someone printed a moved object
        // if (string.data == nullptr)
        // {
        // }

        return ostream << string.data;
    }

    //todo uncomment before sending to progtest (and check all private/public stuff all around)(friend functions)
    private:
    void append(const char* add, const size_t addLen)
    {
        const size_t fullLen = length + addLen;

        if (data == nullptr)
        {
            capacity = fullLen + 1 + (fullLen / 2);

            data = new char[capacity];
        }
        else if (fullLen + 1 > capacity)
        {
            capacity += fullLen + 1 + (fullLen / 2);

            char* tmp = new char[capacity];
            memcpy(tmp, data, length);
            delete [] data;
            data = tmp;
        }

        strncpy(data + length, add, addLen);
        length = fullLen;
        data[length] = '\0';
    }

private:
};

class Address
{
    friend void asserts();
    friend class AddressArr;
    constexpr static char DATE_CHARS = 11;

public:
    char date[DATE_CHARS];
    OwnString street;
    OwnString city;

    Address(const char dateIn[11], OwnString street, OwnString city) : date(), street(std::move(street)),
                                                                       city(std::move(city))
    {
        strncpy(date, dateIn, DATE_CHARS);
    }

    // Address(const char dateIn[11], OwnString street, OwnString city) : street(std::move(street)), city(std::move(city))
    // {
    //     strncpy(date, dateIn, DATE_CHARS);
    // }


    Address(const Address& a) : date(), street(a.street), city(a.city)
    {
        strncpy(date, a.date, DATE_CHARS);
    }

    Address(Address&& a) noexcept : date()
    {
        swapLeftMoved(*this, a);
    }

    ~Address() = default;

    Address& operator=(Address a)
    {
        swapLeftMoved(*this, a);

        return *this;
    }

    static void swapLeftMoved(Address& a, Address& b)
    {
        strncpy(a.date, b.date, DATE_CHARS);
        std::swap(a.street, b.street);
        std::swap(a.city, b.city);
    }

    friend std::ostream& operator<<(std::ostream& os, const Address& a)
    {
        return os << a.date << ' ' << a.street << ' ' << a.city;
    }

private:
    Address() : date{}, street(), city()
    {
    }
};

class AddressArr
{
    friend void asserts();
    Address* data = nullptr;
    size_t count = 0;
    size_t capacity = 0;

public:
    AddressArr() = default;

    AddressArr(const AddressArr& addr) : data(new Address[addr.capacity]), count(addr.count), capacity(addr.capacity)
    {
        for (size_t i = 0; i < addr.count; ++i)
        {
            data[i] = addr.data[i];
        }
    }

    explicit AddressArr(const Address& a) : data(new Address[3]), count(0), capacity(3)
    {
        pushOrdered(a);
    }

    AddressArr(AddressArr&& other) noexcept : data(other.data), count(other.count), capacity(other.capacity)
    {
        other.data = nullptr;
        other.count = 0;
        other.capacity = 0;
    }

    ~AddressArr()
    {
        delete [] data;
        capacity = 0;
    }

    size_t lowerBoundDate(const char date[]) const
    {
        size_t lo = 0, hi = count;
        while (lo < hi)
        {
            const size_t mid = lo + (hi - lo) / 2;
            //when returns < 0, it's a < b
            if (strncmp(data[mid].date, date, Address::DATE_CHARS) < 0)
            {
                lo = mid + 1;
                continue;
            }
            hi = mid;
        }
        return lo;
    }


    bool pushOrdered(Address adr)
    {
        if (data == nullptr)
        {
            capacity = 2;

            data = new Address[capacity];
        }
        else if (count >= capacity)
        {
            capacity = capacity * 2 + 10;
            const auto tmp = new Address[capacity];

            for (size_t i = 0; i < count; ++i)
            {
                tmp[i] = std::move(data[i]);
            }
            delete[] data;
            data = tmp;
        }

        const size_t foundIdx = lowerBoundDate(adr.date);

        if (foundIdx >= count)
        {
            data[count] = std::move(adr);
            ++count;
            return true;
        }

        // the same date resettlement already exists
        if (strncmp(data[foundIdx].date, adr.date, Address::DATE_CHARS) == 0)
        {
            return false;
        }

        Address found = data[foundIdx];
        if (strcmp(data[foundIdx].date, adr.date) == 0)
        {
            return false;
        }

        for (size_t i = count; i > foundIdx; --i)
        {
            auto a = data[i];
            auto b = data[i - 1];
            data[i] = std::move(data[i - 1]);
        }
        ++count;
        data[foundIdx] = std::move(adr);

        return true;
    }

    AddressArr& operator=(AddressArr arr) noexcept
    {
        std::swap(data, arr.data);
        std::swap(count, arr.count);
        std::swap(capacity, arr.capacity);

        return *this;
    }

    const Address& operator[](const size_t index) const
    {
        return data[index];
    }

    Address& operator[](const size_t index)
    {
        return data[index];
    }

    friend std::ostream& operator<<(std::ostream& os, const AddressArr& arr)
    {
        //should never happen but wanna be sure
        if (arr.count == 0) return os;

        for (size_t i = 0; i < arr.count - 1; ++i)
        {
            os << arr.data[i] << "\n";
        }

        return os << arr.data[arr.count - 1];
    }
};

class Person
{
    friend void asserts();
    friend class RowSharedPointer;

public:
    constexpr static char ID_CHARS = 12;
    char id[ID_CHARS]; // possibly {} ?
    OwnString name;
    OwnString surname;
    AddressArr allAddresses;
    size_t refCount = 1;


    Person(const char idIn[7], OwnString name, OwnString surname, const char dateIn[11], OwnString street,
           OwnString city)
        : id(), name(std::move(name)), surname(std::move(surname)),
          allAddresses(Address(dateIn, std::move(street), std::move(city)))
    {
        strncpy(id, idIn, ID_CHARS);
    }

    Person(const Person& p) : id(), name(p.name), surname(p.surname), allAddresses(p.allAddresses), refCount(1)
    {
        strncpy(id, p.id, ID_CHARS);
    }

    Person(Person&& p) noexcept : id(), name(std::move(name)), surname(std::move(surname)),
                                  allAddresses(std::move(allAddresses)), refCount(p.refCount)
    {
        strncpy(id, p.id, ID_CHARS);
        p.refCount = 0;
    }

    ~Person() noexcept = default;

    Person& operator=(Person other) noexcept
    {
        using std::swap;

        strncpy(id, other.id, ID_CHARS);
        swap(name, other.name);
        swap(surname, other.surname);
        swap(allAddresses, other.allAddresses);
        swap(refCount, other.refCount);
        return *this;
    }

    bool resettle(const char date[], const char street[], const char city[])
    {
        return allAddresses.pushOrdered(Address(date, OwnString(street), OwnString(city)));
    }

    bool operator<(const Person& other) const
    {
        return strncmp(id, other.id, ID_CHARS) < 0;
    }
};

class RowSharedPointer
{
    friend void asserts();
    Person* obj;

public:
    RowSharedPointer(): obj(nullptr)
    {
    }

    explicit RowSharedPointer(const Person& p) : obj(new Person(p))
    {
    }

    //todo used to be : obj(new Person(std::move(p)))
    explicit RowSharedPointer(Person&& p) noexcept : obj(new Person(p))
    {
    }

    RowSharedPointer(const RowSharedPointer& row) : obj(row.obj)
    {
        if (obj == nullptr) return;
        obj->refCount++;
    }

    RowSharedPointer(RowSharedPointer&& row) noexcept : obj(row.obj)
    {
        row.obj = nullptr;
    }

    ~RowSharedPointer() noexcept
    {
        if (obj == nullptr) return;
        if (--obj->refCount == 0)
        {
            delete obj;
        }
    }

    RowSharedPointer& operator=(RowSharedPointer right) noexcept
    {
        swap(*this, right);

        return *this;
    }

    bool resettle(const char date[], const char street[], const char city[])
    {
        copyOnWrite();

        return obj->resettle(date, street, city);
    }

    void copyOnWrite()
    {
        if (obj->refCount == 1)
        {
            return;
        }
        --obj->refCount;

        const auto p = new Person(*obj);
        obj = p;
    }

    Person& operator*() const
    {
        return *obj;
    }

    Person* operator ->() const
    {
        return obj;
    }

    bool operator<(const RowSharedPointer& b) const
    {
        return *obj < *b;
    }

    friend std::ostream& operator<<(std::ostream& os, const RowSharedPointer& row)
    {
        return os << row->id << " " << row->name << " " << row->surname << '\n' << row->allAddresses << std::endl;
    }

    static void swap(RowSharedPointer& a, RowSharedPointer& b)
    {
        using std::swap;

        swap(a.obj, b.obj);
    }
};

class CRegister
{
    friend void asserts();
    RowSharedPointer* rowPointers;
    size_t count = 0;
    size_t capacity = 0;

public:
    // default constructor
    CRegister(): rowPointers(nullptr)
    {
    }

    // copy constructor
    CRegister(const CRegister& copy) : rowPointers(new RowSharedPointer[copy.capacity]), count(copy.count),
                                       capacity(copy.capacity)
    {
        for (size_t i = 0; i < copy.count; ++i)
        {
            rowPointers[i] = copy.rowPointers[i];
        }
    }

    // destructor
    ~CRegister()
    {
        delete[] rowPointers;
    }

    // operator =
    CRegister& operator=(CRegister r) noexcept
    {
        std::swap(rowPointers, r.rowPointers);
        std::swap(count, r.count);
        std::swap(capacity, r.capacity);
        return *this;
    }

    size_t size() const
    {
        return count;
    }

    bool add(const char id[], const char name[], const char surname[],
             const char date[], const char street[], const char city[])
    {
        return pushOrdered(RowSharedPointer(Person(id, name, surname, date, street, city)));
    }

    bool resettle(const char id[], const char date[], const char street[], const char city[])
    {
        const size_t foundIdx = lowerBound(id);

        if (foundIdx >= count) return false;

        if (strncmp(rowPointers[foundIdx]->id, id, Person::ID_CHARS) != 0)
        {
            return false;
        }

        return rowPointers[foundIdx].resettle(date, street, city);
    }

    bool print(std::ostream& os, const char id[]) const
    {
        const size_t index = lowerBound(id);

        if (index >= count || strncmp(rowPointers[index]->id, id, Person::ID_CHARS) != 0)
        {
            return false;
        }

        os << rowPointers[index];

        return true;
    }

private:
    void push(const RowSharedPointer& row)
    {
        if (count >= capacity || rowPointers == nullptr)
        {
            capacity = capacity * 2 + 10;
            const auto tmp = new RowSharedPointer[capacity];

            for (size_t i = 0; i < count; ++i)
            {
                tmp[i] = rowPointers[i];
            }
            delete[] rowPointers;
            rowPointers = tmp;
        }
        rowPointers[count++] = row;
    }

    bool pushOrdered(const RowSharedPointer& row)
    {
        if (count >= capacity || rowPointers == nullptr)
        {
            capacity = capacity * 2 + 10;
            const auto tmp = new RowSharedPointer[capacity];

            for (size_t i = 0; i < count; ++i)
            {
                tmp[i] = rowPointers[i];
            }
            delete[] rowPointers;
            rowPointers = tmp;
        }

        const size_t foundIdx = lowerBound(row->id);
        if (foundIdx >= count)
        {
            rowPointers[foundIdx] = row;
            ++count;
            return true;
        }

        if (strncmp(rowPointers[foundIdx]->id, row->id, Person::ID_CHARS) == 0)
        {
            return false;
        }

        for (size_t i = count; i > foundIdx; --i)
        {
            rowPointers[i] = rowPointers[i - 1];
        }
        ++count;
        rowPointers[foundIdx] = row;

        return true;
    }

    size_t lowerBound(const char* targetId) const
    {
        size_t lo = 0, hi = count;
        while (lo < hi)
        {
            const size_t mid = lo + (hi - lo) / 2;
            //when returns < 0, it's a < b
            if (strncmp(rowPointers[mid]->id, targetId, Person::ID_CHARS) < 0)
            {
                lo = mid + 1;
                continue;
            }
            hi = mid;
        }
        return lo;
    }
};

#ifndef __PROGTEST__

void progtestAsserts()
{
    char lID[12], lDate[12], lName[50], lSurname[50], lStreet[50], lCity[50];
    std::ostringstream oss;
    CRegister a;
    assert(a . add ( "123456/7890", "John", "Smith", "2000-01-01", "Main street", "Seattle" ) == true);
    assert(a . add ( "987654/3210", "Freddy", "Kruger", "2001-02-03", "Elm street", "Sacramento" ) == true);
    assert(a . resettle ( "123456/7890", "2003-05-12", "Elm street", "Atlanta" ) == true);
    assert(a . resettle ( "123456/7890", "2002-12-05", "Sunset boulevard", "Los Angeles" ) == true);
    oss.str("");
    assert(a . print ( oss, "123456/7890" ) == true);
    assert(! strcmp ( oss . str () . c_str (), R"###(123456/7890 John Smith
2000-01-01 Main street Seattle
2002-12-05 Sunset boulevard Los Angeles
2003-05-12 Elm street Atlanta
)###" ));
    oss.str("");
    assert(a . print ( oss, "987654/3210" ) == true);
    assert(! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
)###" ));
    CRegister b(a);
    assert(b . resettle ( "987654/3210", "2008-04-12", "Elm street", "Cinccinati" ) == true);
    assert(a . resettle ( "987654/3210", "2007-02-11", "Elm street", "Indianapolis" ) == true);
    oss.str("");
    assert(a . print ( oss, "987654/3210" ) == true);
    assert(! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2007-02-11 Elm street Indianapolis
)###" ));
    oss.str("");
    assert(b . print ( oss, "987654/3210" ) == true);
    assert(! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
)###" ));
    a = b;
    assert(a . resettle ( "987654/3210", "2011-05-05", "Elm street", "Salt Lake City" ) == true);
    oss.str("");
    assert(a . print ( oss, "987654/3210" ) == true);
    assert(! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
2011-05-05 Elm street Salt Lake City
)###" ));
    oss.str("");
    assert(b . print ( oss, "987654/3210" ) == true);
    assert(! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
)###" ));
    assert(b . add ( "987654/3210", "Joe", "Lee", "2010-03-17", "Abbey road", "London" ) == false);
    assert(a . resettle ( "987654/3210", "2001-02-03", "Second street", "Milwaukee" ) == false);
    oss.str("");
    assert(a . print ( oss, "666666/6666" ) == false);

    CRegister c;
    strncpy(lID, "123456/7890", sizeof (lID));
    strncpy(lName, "John", sizeof (lName));
    strncpy(lSurname, "Smith", sizeof (lSurname));
    strncpy(lDate, "2000-01-01", sizeof (lDate));
    strncpy(lStreet, "Main street", sizeof (lStreet));
    strncpy(lCity, "Seattle", sizeof (lCity));
    assert(c . add ( lID, lName, lSurname, lDate, lStreet, lCity ) == true);
    strncpy(lID, "987654/3210", sizeof (lID));
    strncpy(lName, "Freddy", sizeof (lName));
    strncpy(lSurname, "Kruger", sizeof (lSurname));
    strncpy(lDate, "2001-02-03", sizeof (lDate));
    strncpy(lStreet, "Elm street", sizeof (lStreet));
    strncpy(lCity, "Sacramento", sizeof (lCity));
    assert(c . add ( lID, lName, lSurname, lDate, lStreet, lCity ) == true);
    strncpy(lID, "123456/7890", sizeof (lID));
    strncpy(lDate, "2003-05-12", sizeof (lDate));
    strncpy(lStreet, "Elm street", sizeof (lStreet));
    strncpy(lCity, "Atlanta", sizeof (lCity));
    assert(c . resettle ( lID, lDate, lStreet, lCity ) == true);
    strncpy(lID, "123456/7890", sizeof (lID));
    strncpy(lDate, "2002-12-05", sizeof (lDate));
    strncpy(lStreet, "Sunset boulevard", sizeof (lStreet));
    strncpy(lCity, "Los Angeles", sizeof (lCity));
    assert(c . resettle ( lID, lDate, lStreet, lCity ) == true);
    oss.str("");
    assert(c . print ( oss, "123456/7890" ) == true);
    assert(! strcmp ( oss . str () . c_str (), R"###(123456/7890 John Smith
2000-01-01 Main street Seattle
2002-12-05 Sunset boulevard Los Angeles
2003-05-12 Elm street Atlanta
)###" ));
}

void asserts()
{
    {
        OwnString s1("Hello");
        assert(s1.size() == 5);
        assert(std::string(s1.data) == "Hello");

        // Copy Constructor
        OwnString s2(s1);
        assert(s2.size() == s1.size());
        assert(std::string(s2.data) == "Hello");
        assert(std::string(s1.data) == "Hello");

        // Move Constructor
        OwnString s3(std::move(s1));
        assert(s3.size() == 5);
        assert(std::string(s3.data) == "Hello");
        assert(s1.size() == 0);
        assert(s1.data == nullptr);

        // Copy Assignment
        OwnString s4 = s3;
        assert(s4.size() == s3.size());
        assert(std::string(s4.data) == "Hello");
        assert(std::string(s3.data) == "Hello");

        // Move Assignment
        s1 = std::move(s4);
        assert(s1.size() == 5);
        assert(std::string(s1.data) == "Hello");
        assert(s4.data == nullptr);
        assert(s4.size() == 0);

        // Append Function
        s1.append(" World", 6);
        assert(s1.size() == 11);
        assert(std::string(s1.data) == "Hello World");

        std::cout << "OwnString tests passed." << std::endl;
    }

    // -------------------- Test Address --------------------
    {
        Address addr1("2025-03-30", OwnString("Baker Street"), OwnString("London"));
        assert(std::string(addr1.date) == "2025-03-30");
        assert(std::string(addr1.street.data) == "Baker Street");
        assert(std::string(addr1.city.data) == "London");

        // Copy Constructor
        Address addr2(addr1);
        assert(std::string(addr2.date) == "2025-03-30");
        assert(std::string(addr2.street.data) == "Baker Street");
        assert(std::string(addr2.city.data) == "London");

        // Assignment Operator
        Address addr3;
        addr3 = addr1;
        assert(std::string(addr3.date) == "2025-03-30");
        assert(std::string(addr3.street.data) == "Baker Street");
        assert(std::string(addr3.city.data) == "London");

        std::cout << "Address tests passed." << std::endl;
    }

    // -------------------- Test AddressArr --------------------
    {
        AddressArr arr;
        Address addr1("2025-03-30", OwnString("Third Avenue"), OwnString("New York"));
        Address addr2("2024-03-30", OwnString("Second Avenue"), OwnString("New York"));

        assert(arr.pushOrdered(addr1));
        assert(arr.pushOrdered(addr2));
        arr.pushOrdered(Address("2021-03-30", OwnString("First Avenue"), OwnString("New York")));
        assert(arr.pushOrdered(Address("2027-03-30", OwnString("Fourth Avenue"), OwnString("New York"))));
        assert(arr.count == 4);
        Address addDupl("2027-03-30", OwnString("Fourth Avenue"), OwnString("New York"));
        assert(!arr.pushOrdered(std::move(addDupl)));

        // assert(arr[0].street.size() == 10);
        // assert(arr[1].street.size() == 12);
        assert(std::string(arr[0].street.data) == "First Avenue");
        assert(std::string(arr[1].street.data) == "Second Avenue");
        assert(std::string(arr[2].street.data) == "Third Avenue");
        assert(std::string(arr[3].street.data) == "Fourth Avenue");

        // Copy Constructor
        AddressArr arrCopy(arr);
        assert(std::string(arrCopy[0].street.data) == "First Avenue");
        assert(std::string(arrCopy[1].street.data) == "Second Avenue");
        assert(std::string(arrCopy[2].street.data) == "Third Avenue");
        assert(std::string(arrCopy[3].street.data) == "Fourth Avenue");

        // Move Constructor
        AddressArr arrMoved(std::move(arrCopy));
        assert(std::string(arrMoved[0].street.data) == "First Avenue");
        assert(std::string(arrMoved[1].street.data) == "Second Avenue");
        assert(std::string(arrMoved[2].street.data) == "Third Avenue");
        assert(std::string(arrMoved[3].street.data) == "Fourth Avenue");
        // assert(arr[0].street.data == nullptr);  // Moved-from array should be empty

        std::cout << "AddressArr tests passed." << std::endl;
    }

    // -------------------- Test Person --------------------
    {
        Person person("ABC123", OwnString("Sherlock"), OwnString("Holmes"), "2025-03-30", OwnString("Baker Street"),
                      OwnString("London"));
        assert(std::string(person.id) == "ABC123");
        assert(std::string(person.name.data) == "Sherlock");
        assert(std::string(person.surname.data) == "Holmes");

        const Address& addr = person.allAddresses[0];
        assert(std::string(addr.date) == "2025-03-30");
        assert(std::string(addr.street.data) == "Baker Street");
        assert(std::string(addr.city.data) == "London");

        // Copy Constructor
        Person personCopy(person);
        assert(std::string(personCopy.id) == "ABC123");
        assert(std::string(personCopy.name.data) == "Sherlock");
        assert(std::string(personCopy.surname.data) == "Holmes");
        assert(std::string(personCopy.allAddresses[0].street.data) == "Baker Street");

        std::cout << "Person tests passed." << std::endl;
    }

    {
        Person p1("ABC123/1234", "Alice", "Wonder", "2025-01-01", "Street1", "City1");
        Person p2("BCD234/1234", "Bob", "Marley", "2025-02-01", "Street2", "City2");
        Person p3("ABC123/1234", "Carol", "Danvers", "2025-03-01", "Street3", "City3");
        Person p4("AAA111/1234", "Dave", "Smith", "2025-04-01", "Street4", "City4");

        // Test operator< using strncmp on fixed-length IDs.
        assert(p1 < p2);
        assert(!(p2 < p1));

        // Identical IDs: neither is less than the other.
        assert(std::strncmp(p1.id, p3.id, Person::ID_CHARS) == 0);
        assert(!(p1 < p3));
        assert(!(p3 < p1));

        // Lexicographically smaller test.
        assert(p4 < p1);
        assert(!(p1 < p4));

        assert(strcmp(p1.name.data, "Alice") == 0);
        assert(strcmp(p2.surname.data, "Marley") == 0);
        assert(strcmp(p3.allAddresses[0].date, "2025-03-01") == 0);
        assert(strcmp(p2.allAddresses[0].street.data, "Street2") == 0);
        assert(strcmp(p4.allAddresses[0].city.data, "City4") == 0);
    }
    std::cout << "Person tests passed." << std::endl;

    {
        // Create a Person.
        Person p1("XYZ789/1234", "John", "Doe", "2025-03-30", "Main St", "Town");

        // Create a RowSharedPointer managing p1.
        RowSharedPointer rsp1(p1);
        {
            // Create a copy of rsp1. Both should share the same Person.
            RowSharedPointer rsp2(rsp1);
            // Verify that the refCount has increased to 2.
            assert(rsp1->refCount == 2);
            assert(rsp2->refCount == 2);
        }
        // After rsp2 goes out of scope, refCount should drop back to 1.
        assert(rsp1->refCount == 1);

        assert(strcmp(rsp1->name.data, "John") == 0);
        assert(strcmp(rsp1->surname.data, "Doe") == 0);
        assert(strcmp(rsp1->allAddresses[0].date, "2025-03-30") == 0);
        assert(strcmp(rsp1->allAddresses[0].street.data, "Main St") == 0);
        assert(strcmp(rsp1->allAddresses[0].city.data, "Town") == 0);

        // Test move construction: transferring ownership.
        RowSharedPointer rsp3(std::move(rsp1));
        // rsp3 now owns the object with a refCount of 1.
        assert(rsp3->refCount == 1);

        assert(strcmp(rsp3->name.data, "John") == 0);
        assert(strcmp(rsp3->surname.data, "Doe") == 0);
        assert(strcmp(rsp3->allAddresses[0].date, "2025-03-30") == 0);
        assert(strcmp(rsp3->allAddresses[0].street.data, "Main St") == 0);
        assert(strcmp(rsp3->allAddresses[0].city.data, "Town") == 0);
    }
    std::cout << "RowSharedPointer tests passed." << std::endl;

    {
        CRegister reg;
        // Add several Persons to the register.
        reg.add("BBB222/1234", "Bob", "Brown", "2025-02-01", "Second St", "CityB");
        reg.add("CCC333/1234", "Charlie", "Davis", "2025-03-01", "Third St", "CityC");
        reg.add("AAA111/1234", "Alice", "Smith", "2025-01-01", "Third St", "CityA");

        // Verify that the register has 3 elements.
        assert(reg.size() == 3);
        // Check that the IDs are stored correctly. In correct order
        assert(std::strncmp(reg.rowPointers[0]->id, "AAA111/1234", Person::ID_CHARS) == 0);
        assert(std::strncmp(reg.rowPointers[1]->id, "BBB222/1234", Person::ID_CHARS) == 0);
        assert(std::strncmp(reg.rowPointers[2]->id, "CCC333/1234", Person::ID_CHARS) == 0);


        assert(std::strncmp(reg.rowPointers[0]->name.data, "Alice", Person::ID_CHARS) == 0);
        assert(std::strncmp(reg.rowPointers[1]->surname.data, "Brown", Person::ID_CHARS) == 0);
        assert(std::strncmp(reg.rowPointers[2]->allAddresses[0].date , "2025-03-01", Person::ID_CHARS) == 0);
        assert(std::strncmp(reg.rowPointers[2]->allAddresses[0].street.data, "Third St", Person::ID_CHARS) == 0);
        assert(std::strncmp(reg.rowPointers[2]->allAddresses[0].city.data, "CityC", Person::ID_CHARS) == 0);

        // Copy the register and verify that the copied register contains the same data.
        CRegister regCopy(reg);
        assert(regCopy.size() == 3);
        assert(std::strncmp(regCopy.rowPointers[0]->id, "AAA111/1234", Person::ID_CHARS) == 0);
        assert(regCopy.rowPointers[0]->refCount == 2);
        assert(std::strncmp(regCopy.rowPointers[1]->id, "BBB222/1234", Person::ID_CHARS) == 0);
        assert(std::strncmp(regCopy.rowPointers[2]->id, "CCC333/1234", Person::ID_CHARS) == 0);
    }
    std::cout << "CRegister tests passed." << std::endl;

    {
        CRegister reg;

        // Test resettle
        assert(reg.add("000001/1234", "John", "Doe", "2025-03-30", "First Street", "New York"));
        assert(reg.add("000002/1234", "Jane", "Smith", "2025-03-29", "Second Street", "Los Angeles"));

        // Successful resettlement
        assert(reg.resettle("000001/1234", "2025-03-31", "Third Street", "Chicago"));

        // Unsuccessful resettlement - same date
        assert(!reg.resettle("000001/1234", "2025-03-31", "Fourth Street", "Boston"));

        // Unsuccessful resettlement - non-existent person
        assert(!reg.resettle("000003/1234", "2025-04-01", "Fifth Street", "San Francisco"));

        // Test print
        std::ostringstream oss;

        // Successful print
        assert(reg.print(oss, "000001/1234"));
        std::string expectedOutput =
            "000001/1234 John Doe\n2025-03-30 First Street New York\n2025-03-31 Third Street Chicago\n";
        assert(oss.str() == expectedOutput);

        oss.str(""); // Clear the stream

        // Unsuccessful print - non-existent person
        assert(!reg.print(oss, "000003/1234"));

        std::cout << "All asserts passed successfully." << std::endl;
    }

    std::cout << "All tests passed successfully!" << std::endl;
}



int main()
{
    asserts();
    progtestAsserts();
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
