#ifndef __PROGTEST__
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <memory>
#include <functional>
#include <stdexcept>
#endif /* __PROGTEST__ */

class CTableElement
{
public:
    virtual ~CTableElement() = default;
    [[nodiscard]] virtual std::vector<std::string> print(int targetWidth, int targetHeight) const = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual CTableElement* clone() const = 0;
    virtual bool equals(const CTableElement& o) const = 0;

    bool operator==(const CTableElement& o) const
    {
        return this->equals(o);
    }
    bool operator!=(const CTableElement& o) const
    {
        return !(*this == o);
    }
};


class CEmpty : public CTableElement
{
public:
    CEmpty() = default;
    ~CEmpty() override = default;
    CEmpty(const CEmpty& copy) = default;

    CTableElement* clone() const override
    {
        return new CEmpty(*this);
    }

    bool equals(const CTableElement& o) const override
    {
        if (dynamic_cast<const CEmpty*>(&o))
        {
            return true;
        }
        return false;
    }

    [[nodiscard]] std::vector<std::string> print(const int targetWidth, const int targetHeight) const override
    {
        std::vector<std::string> ret{};

        for (int i = 0; i < targetHeight; ++i)
        {
            ret.emplace_back(targetWidth, ' ');
        }
        return ret;
    }


    int getWidth() const override
    {
        return 0;
    }

    int getHeight() const override
    {
        return 0;
    }


};


class CText : public CTableElement
{
public:
    enum Align
    {
        ALIGN_LEFT,
        ALIGN_RIGHT
    };

private:
    std::vector<std::string> rows;
    Align align;

    int width = 0;

public:
    CText(const std::string& str, const Align align): rows(), align(align)
    {
        setText(str);
    }
    CText(const CText& copy): rows(copy.rows), align(copy.align), width(copy.width)
    {

    }

    ~CText() override = default;

    CTableElement* clone() const override
    {
        return new CText(*this);
    }

    bool equals(const CTableElement& o) const override
    {
        if (const auto ptrO = dynamic_cast<const CText*>(&o))
        {

            if (ptrO->rows.size() != rows.size() || ptrO->align != align)
            {
                return false;
            }
            const size_t n = rows.size();
            for (size_t i = 0; i < n; ++i)
            {
                if (rows[i] != ptrO->rows[i])
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    [[nodiscard]] std::vector<std::string> print(const int targetWidth, const int targetHeight) const override
    {
        return align == ALIGN_LEFT
                   ? printAlignLeft(targetWidth, targetHeight)
                   : printAlignRight(targetWidth, targetHeight);
    }

    int getWidth() const override
    {
        return width;
    }

    int getHeight() const override
    {
        return static_cast<int>(rows.size());
    }

    CText& setText(const std::string& str)
    {
        rows.clear();
        width = 0;

        int lineStart = 0;
        int curWidth = 0;
        for (const char c : str)
        {
            //split by newLines, find the width of the cell
            if (c == '\n')
            {
                rows.push_back(str.substr(lineStart, curWidth));
                lineStart += curWidth + 1;
                width = curWidth > width ? curWidth : width;
                curWidth = 0;
                continue;
            }
            ++curWidth;
        }
        width = curWidth > width ? curWidth : width;
        rows.push_back(str.substr(lineStart, curWidth));

        return *this;
    }

private:
    [[nodiscard]] std::vector<std::string> printAlignLeft(const int targetWidth, const int targetHeight) const
    {
        std::vector<std::string> ret{};

        for (const auto& row : rows)
        {
            ret.push_back(row + std::string(targetWidth - row.size(), ' '));
        }

        for (int i = static_cast<int>(rows.size()); i < targetHeight; ++i)
        {
            ret.emplace_back(targetWidth, ' ');
        }

        return ret;
    }

    [[nodiscard]] std::vector<std::string> printAlignRight(const int targetWidth, const int targetHeight) const
    {
        std::vector<std::string> ret{};

        for (const auto& row : rows)
        {
            ret.push_back(std::string(targetWidth - row.size(), ' ') + row);
        }

        for (int i = static_cast<int>(rows.size()); i < targetHeight; ++i)
        {
            ret.emplace_back(targetWidth, ' ');
        }

        return ret;
    }
};

class CImage : public CTableElement
{
    std::vector<std::string> rows;

public:
    CImage() = default;

    CImage(const CImage& im) = default;

    ~CImage() override = default;

    CTableElement* clone() const override
    {
        return new CImage(*this);
    }

    bool equals(const CTableElement& o) const override
    {
        if (const auto ptrO = dynamic_cast<const CImage*>(&o))
        {
            if (ptrO->rows.size() != rows.size())
            {
                return false;
            }
            const size_t n = rows.size();
            for (size_t i = 0; i < n; ++i)
            {
                if (rows[i] != ptrO->rows[i])
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    [[nodiscard]] std::vector<std::string> print(const int targetWidth, const int targetHeight) const override
    {
        std::vector<std::string> ret{};

        if (rows.empty())
        {
            for (int i = static_cast<int>(rows.size()); i < targetHeight; ++i)
            {
                ret.emplace_back(targetWidth, ' ');
            }
            return ret;
        }

        //images must be centered

        int remH = targetHeight - static_cast<int>(rows.size());
        int hi;
        int lo = hi = remH / 2;
        lo = remH % 2 == 1 ? lo + 1 : lo;

        //padding
        for (int i = 0; i < hi; ++i)
        {
            ret.emplace_back(targetWidth, ' ');
        }

        const int remW = targetWidth - static_cast<int>(rows[0].size());
        int ri;
        int le = ri = remW / 2;
        ri = remW % 2 == 1 ? ri + 1 : ri;
        for (const auto& row : rows)
        {
            ret.push_back(std::string(le, ' ') + row + std::string(ri, ' '));
        }

        //padding
        for (int i = 0; i < lo; ++i)
        {
            ret.emplace_back(targetWidth, ' ');
        }

        return ret;
    }

    int getWidth() const override
    {
        if (rows.size() > 1)
        {
            return static_cast<int>(rows[0].size());
        }
        return 0;
    }

    int getHeight() const override
    {
        return static_cast<int>(rows.size());
    }

    CImage& addRow(const std::string& str)
    {
        if (rows.size() > 1 && rows[0].size() != str.size())
        {
            throw std::invalid_argument("Image rows aren't the same size");
        }

        rows.push_back(str);

        return *this;
    }
};


// using UTableElement = std::unique_ptr<CTableElement>;

class UTableElement
{
    CTableElement * ptr;
public:
    explicit UTableElement(CTableElement* ptr) : ptr(ptr) {}
    ~UTableElement()
    {
        delete ptr;
    }
    UTableElement (const UTableElement & cp) : ptr(cp->clone()) {}
    UTableElement (UTableElement && mv) noexcept : ptr(mv.ptr)
    {
        mv.ptr = nullptr;
    }
    UTableElement & operator=(UTableElement copy) noexcept
    {
        std::swap(ptr, copy.ptr);
        return *this;
    }
    CTableElement * operator->() const
    {
        return ptr;
    }
    CTableElement & operator*() const
    {
        return *ptr;
    }
};

class CTable : public CTableElement
{
    int rows, cols;
    std::vector<std::vector<UTableElement>> elements;

public:
    CTable(const int rows, const int cols) : rows(rows), cols(cols)
    {
        if (rows <= 0 || cols <= 0) {
            throw std::invalid_argument("Table can't have 0 rows or 0 columns");
        }

        elements.resize(rows);

        for (int row = 0; row < rows; ++row)
        {
            // elements[row].reserve(cols); // pre-allocates at least cols elements
            for (int col = 0; col < cols; ++col)
            {
                elements[row].emplace_back(new CEmpty());
            }
        }
    }

    CTable(const CTable& copy) : rows(copy.rows), cols(copy.cols), elements(copy.elements)
    {

    }

    CTable& operator=(CTable copy)
    {
        std::swap(rows, copy.rows);
        std::swap(cols, copy.cols);
        std::swap(elements, copy.elements);

        return *this;
    }

    bool operator==(const CTable& o) const
    {
        if (rows != o.rows || cols != o.cols) return false;

        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                if (*elements[row][col] != *o.elements[row][col])
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool operator!=(const CTable& o) const
    {
        return !(*this == o);
    }

    friend std::ostream& operator<<(std::ostream& os, const CTable& table)
    {
        if (table.rows <= 0 || table.cols <= 0)
        {
            return os;
        }

        const auto [colWidths, rowHeights] = table.getTargetWidthHeight();
        // int totalWidth = 0;
        // for (const int w : colWidths) {totalWidth += w;}


        for (size_t row = 0; row < table.elements.size(); ++row)
        {
            const size_t n = table.elements[row].size();

            std::vector<std::vector<std::string>> printStrings;
            for (size_t col = 0; col < n; ++col)
            {
                printStrings.push_back(table.elements[row][col]->print(colWidths[col], rowHeights[row]));
            }

            printBoundary(os, colWidths);
            os << std::endl;

            for (int line = 0; line < rowHeights[row]; ++line)
            {

                for (int cell = 0; cell < table.cols; ++cell)
                {
                    os << "|" << printStrings[cell][line];
                }
                os << "|" << std::endl;
            }
            //table boundary
        }
        printBoundary(os, colWidths);
        os << std::endl;

        return os;
    }

    CTableElement& getCell(const int row, const int col)
    {
        if (row < 0 || col < 0 || row >= rows || col >= cols)
        {
            throw std::out_of_range("Indexes out of table's bounds");
        }
        return *elements[row][col];
    }

    CTableElement& setCell(const int row, const int col, const CTableElement& el)
    {
        elements[row][col] = UTableElement(el.clone());
        return *elements[row][col];
    }

private:

    static void printBoundary(std::ostream& os, const std::vector<int>& colWidths)
    {
        for (const int colWidth : colWidths)
        {
            os << '+' << std::string(colWidth, '-');
        }
        os << '+';;
    }

    std::pair<std::vector<int>, std::vector<int>> getTargetWidthHeight() const
    {
        if (elements.empty())
        {
            return {std::vector<int>(), std::vector<int>()};
        }
        std::vector<int> colWidths(cols);
        std::vector<int> rowHeights(rows);

        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                int w = elements[row][col]->getWidth();
                colWidths[col] = w > colWidths[col] ? w : colWidths[col];

                int h = elements[row][col]->getHeight();
                rowHeights[row] = h > rowHeights[row] ? h : rowHeights[row];
            }
        }

        return {colWidths, rowHeights};
    }

public:
    [[nodiscard]] std::vector<std::string> print(int targetWidth, int targetHeight) const override {
    //todo missing padding on the right, bottom
        std::vector<std::string> ret{};


        const auto [colWidths, rowHeights] = getTargetWidthHeight();
        // int totalWidth = 0;
        // for (const int w : colWidths) {totalWidth += w;}


        for (size_t row = 0; row < elements.size(); ++row)
        {
            const size_t n = elements[row].size();

            std::vector<std::vector<std::string>> printStrings;
            for (size_t col = 0; col < n; ++col)
            {
                printStrings.push_back(elements[row][col]->print(colWidths[col], rowHeights[row]));
            }

            std::ostringstream strStream{};
            printBoundary(strStream, colWidths);
            ret.push_back(std::move(strStream.str()));

            strStream.clear();


            for (int line = 0; line < rowHeights[row]; ++line)
            {

                for (int cell = 0; cell < cols; ++cell)
                {
                    strStream << "|" << printStrings[cell][line];
                }
                strStream << "|";
                ret.push_back(std::move(strStream.str()));
                strStream.clear();
            }
            //table boundary
        }
        std::ostringstream strStream{};
        printBoundary(strStream, colWidths);
        ret.push_back(std::move(strStream.str()));

        return ret;
    }

    int getWidth() const override {
        //todo make it more effective
        const auto [widths, heights] = getTargetWidthHeight();
        int totalWidth = 1; //starting border
        for (const int width: widths) {
            totalWidth += width + 1; //+1 border
        }
        return totalWidth;
    }

    int getHeight() const override {
        const auto [widths, heights] = getTargetWidthHeight();

        int totalHeight = 1;

        for (const int height: heights) {
            totalHeight += height + 1;
        }

        return totalHeight;
    }

    CTableElement* clone() const override {
        return new CTable(*this);
    }

    bool equals(const CTableElement &o) const override {
        if (const auto ptrO = dynamic_cast<const CTable*>(&o)) {
            return *this == *ptrO;
        }

        return false;
    }
};

#ifndef __PROGTEST__
int main ()
{
  std::ostringstream oss;
  CTable t0 ( 3, 2 );
  t0 . setCell ( 0, 0, CText ( "Hello,\n"
        "Hello Kitty", CText::ALIGN_LEFT ) );
  t0 . setCell ( 1, 0, CText ( "Lorem ipsum dolor sit amet", CText::ALIGN_LEFT ) );
  t0 . setCell ( 2, 0, CText ( "Bye,\n"
        "Hello Kitty", CText::ALIGN_RIGHT ) );
  t0 . setCell ( 1, 1, CImage ()
          . addRow ( "###                   " )
          . addRow ( "#  #                  " )
          . addRow ( "#  # # ##   ###    ###" )
          . addRow ( "###  ##    #   #  #  #" )
          . addRow ( "#    #     #   #  #  #" )
          . addRow ( "#    #     #   #  #  #" )
          . addRow ( "#    #      ###    ###" )
          . addRow ( "                     #" )
          . addRow ( "                   ## " )
          . addRow ( "                      " )
          . addRow ( " #    ###   ###   #   " )
          . addRow ( "###  #   # #     ###  " )
          . addRow ( " #   #####  ###   #   " )
          . addRow ( " #   #         #  #   " )
          . addRow ( "  ##  ###   ###    ## " ) );
  t0 . setCell ( 2, 1, CEmpty () );
  oss . str ("");
  oss . clear ();
  oss << t0;
  assert ( oss . str () ==
        "+--------------------------+----------------------+\n"
        "|Hello,                    |                      |\n"
        "|Hello Kitty               |                      |\n"
        "+--------------------------+----------------------+\n"
        "|Lorem ipsum dolor sit amet|###                   |\n"
        "|                          |#  #                  |\n"
        "|                          |#  # # ##   ###    ###|\n"
        "|                          |###  ##    #   #  #  #|\n"
        "|                          |#    #     #   #  #  #|\n"
        "|                          |#    #     #   #  #  #|\n"
        "|                          |#    #      ###    ###|\n"
        "|                          |                     #|\n"
        "|                          |                   ## |\n"
        "|                          |                      |\n"
        "|                          | #    ###   ###   #   |\n"
        "|                          |###  #   # #     ###  |\n"
        "|                          | #   #####  ###   #   |\n"
        "|                          | #   #         #  #   |\n"
        "|                          |  ##  ###   ###    ## |\n"
        "+--------------------------+----------------------+\n"
        "|                      Bye,|                      |\n"
        "|               Hello Kitty|                      |\n"
        "+--------------------------+----------------------+\n" );
  t0 . setCell ( 0, 1, t0 . getCell ( 1, 1 ) );
  t0 . setCell ( 2, 1, CImage ()
          . addRow ( "*****   *      *  *      ******* ******  *" )
          . addRow ( "*    *  *      *  *      *            *  *" )
          . addRow ( "*    *  *      *  *      *           *   *" )
          . addRow ( "*    *  *      *  *      *****      *    *" )
          . addRow ( "****    *      *  *      *         *     *" )
          . addRow ( "*  *    *      *  *      *        *       " )
          . addRow ( "*   *   *      *  *      *       *       *" )
          . addRow ( "*    *    *****   ****** ******* ******  *" ) );
  dynamic_cast<CText &> ( t0 . getCell ( 1, 0 ) ) . setText ( "Lorem ipsum dolor sit amet,\n"
        "consectetur adipiscing\n"
        "elit. Curabitur scelerisque\n"
        "lorem vitae lectus cursus,\n"
        "vitae porta ante placerat. Class aptent taciti\n"
        "sociosqu ad litora\n"
        "torquent per\n"
        "conubia nostra,\n"
        "per inceptos himenaeos.\n"
        "\n"
        "Donec tincidunt augue\n"
        "sit amet metus\n"
        "pretium volutpat.\n"
        "Donec faucibus,\n"
        "ante sit amet\n"
        "luctus posuere,\n"
        "mauris tellus" );
  oss . str ("");
  oss . clear ();
  oss << t0;
  assert ( oss . str () ==
        "+----------------------------------------------+------------------------------------------+\n"
        "|Hello,                                        |          ###                             |\n"
        "|Hello Kitty                                   |          #  #                            |\n"
        "|                                              |          #  # # ##   ###    ###          |\n"
        "|                                              |          ###  ##    #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #      ###    ###          |\n"
        "|                                              |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|                                              |                                          |\n"
        "|                                              |           #    ###   ###   #             |\n"
        "|                                              |          ###  #   # #     ###            |\n"
        "|                                              |           #   #####  ###   #             |\n"
        "|                                              |           #   #         #  #             |\n"
        "|                                              |            ##  ###   ###    ##           |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|Lorem ipsum dolor sit amet,                   |                                          |\n"
        "|consectetur adipiscing                        |          ###                             |\n"
        "|elit. Curabitur scelerisque                   |          #  #                            |\n"
        "|lorem vitae lectus cursus,                    |          #  # # ##   ###    ###          |\n"
        "|vitae porta ante placerat. Class aptent taciti|          ###  ##    #   #  #  #          |\n"
        "|sociosqu ad litora                            |          #    #     #   #  #  #          |\n"
        "|torquent per                                  |          #    #     #   #  #  #          |\n"
        "|conubia nostra,                               |          #    #      ###    ###          |\n"
        "|per inceptos himenaeos.                       |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|Donec tincidunt augue                         |                                          |\n"
        "|sit amet metus                                |           #    ###   ###   #             |\n"
        "|pretium volutpat.                             |          ###  #   # #     ###            |\n"
        "|Donec faucibus,                               |           #   #####  ###   #             |\n"
        "|ante sit amet                                 |           #   #         #  #             |\n"
        "|luctus posuere,                               |            ##  ###   ###    ##           |\n"
        "|mauris tellus                                 |                                          |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
        "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|                                              |*    *  *      *  *      *           *   *|\n"
        "|                                              |*    *  *      *  *      *****      *    *|\n"
        "|                                              |****    *      *  *      *         *     *|\n"
        "|                                              |*  *    *      *  *      *        *       |\n"
        "|                                              |*   *   *      *  *      *       *       *|\n"
        "|                                              |*    *    *****   ****** ******* ******  *|\n"
        "+----------------------------------------------+------------------------------------------+\n" );
  CTable t1 ( t0 );
  t1 . setCell ( 1, 0, CEmpty () );
  t1 . setCell ( 1, 1, CEmpty () );
  oss . str ("");
  oss . clear ();
  oss << t0;
  assert ( oss . str () ==
        "+----------------------------------------------+------------------------------------------+\n"
        "|Hello,                                        |          ###                             |\n"
        "|Hello Kitty                                   |          #  #                            |\n"
        "|                                              |          #  # # ##   ###    ###          |\n"
        "|                                              |          ###  ##    #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #      ###    ###          |\n"
        "|                                              |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|                                              |                                          |\n"
        "|                                              |           #    ###   ###   #             |\n"
        "|                                              |          ###  #   # #     ###            |\n"
        "|                                              |           #   #####  ###   #             |\n"
        "|                                              |           #   #         #  #             |\n"
        "|                                              |            ##  ###   ###    ##           |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|Lorem ipsum dolor sit amet,                   |                                          |\n"
        "|consectetur adipiscing                        |          ###                             |\n"
        "|elit. Curabitur scelerisque                   |          #  #                            |\n"
        "|lorem vitae lectus cursus,                    |          #  # # ##   ###    ###          |\n"
        "|vitae porta ante placerat. Class aptent taciti|          ###  ##    #   #  #  #          |\n"
        "|sociosqu ad litora                            |          #    #     #   #  #  #          |\n"
        "|torquent per                                  |          #    #     #   #  #  #          |\n"
        "|conubia nostra,                               |          #    #      ###    ###          |\n"
        "|per inceptos himenaeos.                       |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|Donec tincidunt augue                         |                                          |\n"
        "|sit amet metus                                |           #    ###   ###   #             |\n"
        "|pretium volutpat.                             |          ###  #   # #     ###            |\n"
        "|Donec faucibus,                               |           #   #####  ###   #             |\n"
        "|ante sit amet                                 |           #   #         #  #             |\n"
        "|luctus posuere,                               |            ##  ###   ###    ##           |\n"
        "|mauris tellus                                 |                                          |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
        "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|                                              |*    *  *      *  *      *           *   *|\n"
        "|                                              |*    *  *      *  *      *****      *    *|\n"
        "|                                              |****    *      *  *      *         *     *|\n"
        "|                                              |*  *    *      *  *      *        *       |\n"
        "|                                              |*   *   *      *  *      *       *       *|\n"
        "|                                              |*    *    *****   ****** ******* ******  *|\n"
        "+----------------------------------------------+------------------------------------------+\n" );
  oss . str ("");

    std::cout << t1;
  oss . clear ();
  oss << t1;
  assert ( oss . str () ==
        "+-----------+------------------------------------------+\n"
        "|Hello,     |          ###                             |\n"
        "|Hello Kitty|          #  #                            |\n"
        "|           |          #  # # ##   ###    ###          |\n"
        "|           |          ###  ##    #   #  #  #          |\n"
        "|           |          #    #     #   #  #  #          |\n"
        "|           |          #    #     #   #  #  #          |\n"
        "|           |          #    #      ###    ###          |\n"
        "|           |                               #          |\n"
        "|           |                             ##           |\n"
        "|           |                                          |\n"
        "|           |           #    ###   ###   #             |\n"
        "|           |          ###  #   # #     ###            |\n"
        "|           |           #   #####  ###   #             |\n"
        "|           |           #   #         #  #             |\n"
        "|           |            ##  ###   ###    ##           |\n"
        "+-----------+------------------------------------------+\n"
        "+-----------+------------------------------------------+\n"
        "|       Bye,|*****   *      *  *      ******* ******  *|\n"
        "|Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|           |*    *  *      *  *      *           *   *|\n"
        "|           |*    *  *      *  *      *****      *    *|\n"
        "|           |****    *      *  *      *         *     *|\n"
        "|           |*  *    *      *  *      *        *       |\n"
        "|           |*   *   *      *  *      *       *       *|\n"
        "|           |*    *    *****   ****** ******* ******  *|\n"
        "+-----------+------------------------------------------+\n" );
  t1 = t0;
  t1 . setCell ( 0, 0, CEmpty () );
  t1 . setCell ( 1, 1, CImage ()
          . addRow ( "  ********                    " )
          . addRow ( " **********                   " )
          . addRow ( "**        **                  " )
          . addRow ( "**             **        **   " )
          . addRow ( "**             **        **   " )
          . addRow ( "***         ********  ********" )
          . addRow ( "****        ********  ********" )
          . addRow ( "****           **        **   " )
          . addRow ( "****           **        **   " )
          . addRow ( "****      **                  " )
          . addRow ( " **********                   " )
          . addRow ( "  ********                    " ) );
  oss . str ("");
  oss . clear ();
  oss << t0;
  assert ( oss . str () ==
        "+----------------------------------------------+------------------------------------------+\n"
        "|Hello,                                        |          ###                             |\n"
        "|Hello Kitty                                   |          #  #                            |\n"
        "|                                              |          #  # # ##   ###    ###          |\n"
        "|                                              |          ###  ##    #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #      ###    ###          |\n"
        "|                                              |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|                                              |                                          |\n"
        "|                                              |           #    ###   ###   #             |\n"
        "|                                              |          ###  #   # #     ###            |\n"
        "|                                              |           #   #####  ###   #             |\n"
        "|                                              |           #   #         #  #             |\n"
        "|                                              |            ##  ###   ###    ##           |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|Lorem ipsum dolor sit amet,                   |                                          |\n"
        "|consectetur adipiscing                        |          ###                             |\n"
        "|elit. Curabitur scelerisque                   |          #  #                            |\n"
        "|lorem vitae lectus cursus,                    |          #  # # ##   ###    ###          |\n"
        "|vitae porta ante placerat. Class aptent taciti|          ###  ##    #   #  #  #          |\n"
        "|sociosqu ad litora                            |          #    #     #   #  #  #          |\n"
        "|torquent per                                  |          #    #     #   #  #  #          |\n"
        "|conubia nostra,                               |          #    #      ###    ###          |\n"
        "|per inceptos himenaeos.                       |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|Donec tincidunt augue                         |                                          |\n"
        "|sit amet metus                                |           #    ###   ###   #             |\n"
        "|pretium volutpat.                             |          ###  #   # #     ###            |\n"
        "|Donec faucibus,                               |           #   #####  ###   #             |\n"
        "|ante sit amet                                 |           #   #         #  #             |\n"
        "|luctus posuere,                               |            ##  ###   ###    ##           |\n"
        "|mauris tellus                                 |                                          |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
        "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|                                              |*    *  *      *  *      *           *   *|\n"
        "|                                              |*    *  *      *  *      *****      *    *|\n"
        "|                                              |****    *      *  *      *         *     *|\n"
        "|                                              |*  *    *      *  *      *        *       |\n"
        "|                                              |*   *   *      *  *      *       *       *|\n"
        "|                                              |*    *    *****   ****** ******* ******  *|\n"
        "+----------------------------------------------+------------------------------------------+\n" );
  oss . str ("");
  oss . clear ();
  oss << t1;
  assert ( oss . str () ==
        "+----------------------------------------------+------------------------------------------+\n"
        "|                                              |          ###                             |\n"
        "|                                              |          #  #                            |\n"
        "|                                              |          #  # # ##   ###    ###          |\n"
        "|                                              |          ###  ##    #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #      ###    ###          |\n"
        "|                                              |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|                                              |                                          |\n"
        "|                                              |           #    ###   ###   #             |\n"
        "|                                              |          ###  #   # #     ###            |\n"
        "|                                              |           #   #####  ###   #             |\n"
        "|                                              |           #   #         #  #             |\n"
        "|                                              |            ##  ###   ###    ##           |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|Lorem ipsum dolor sit amet,                   |                                          |\n"
        "|consectetur adipiscing                        |                                          |\n"
        "|elit. Curabitur scelerisque                   |        ********                          |\n"
        "|lorem vitae lectus cursus,                    |       **********                         |\n"
        "|vitae porta ante placerat. Class aptent taciti|      **        **                        |\n"
        "|sociosqu ad litora                            |      **             **        **         |\n"
        "|torquent per                                  |      **             **        **         |\n"
        "|conubia nostra,                               |      ***         ********  ********      |\n"
        "|per inceptos himenaeos.                       |      ****        ********  ********      |\n"
        "|                                              |      ****           **        **         |\n"
        "|Donec tincidunt augue                         |      ****           **        **         |\n"
        "|sit amet metus                                |      ****      **                        |\n"
        "|pretium volutpat.                             |       **********                         |\n"
        "|Donec faucibus,                               |        ********                          |\n"
        "|ante sit amet                                 |                                          |\n"
        "|luctus posuere,                               |                                          |\n"
        "|mauris tellus                                 |                                          |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
        "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|                                              |*    *  *      *  *      *           *   *|\n"
        "|                                              |*    *  *      *  *      *****      *    *|\n"
        "|                                              |****    *      *  *      *         *     *|\n"
        "|                                              |*  *    *      *  *      *        *       |\n"
        "|                                              |*   *   *      *  *      *       *       *|\n"
        "|                                              |*    *    *****   ****** ******* ******  *|\n"
        "+----------------------------------------------+------------------------------------------+\n" );
  CTable t2 ( 2, 2 );
  t2 . setCell ( 0, 0, CText ( "OOP", CText::ALIGN_LEFT ) );
  t2 . setCell ( 0, 1, CText ( "Encapsulation", CText::ALIGN_LEFT ) );
  t2 . setCell ( 1, 0, CText ( "Polymorphism", CText::ALIGN_LEFT ) );
  t2 . setCell ( 1, 1, CText ( "Inheritance", CText::ALIGN_LEFT ) );
  oss . str ("");
  oss . clear ();
  oss << t2;
  assert ( oss . str () ==
        "+------------+-------------+\n"
        "|OOP         |Encapsulation|\n"
        "+------------+-------------+\n"
        "|Polymorphism|Inheritance  |\n"
        "+------------+-------------+\n" );
  t1 . setCell ( 0, 0, t2 );
  dynamic_cast<CText &> ( t2 . getCell ( 0, 0 ) ) . setText ( "Object Oriented Programming" );
  oss . str ("");
  oss . clear ();
  oss << t2;
  assert ( oss . str () ==
        "+---------------------------+-------------+\n"
        "|Object Oriented Programming|Encapsulation|\n"
        "+---------------------------+-------------+\n"
        "|Polymorphism               |Inheritance  |\n"
        "+---------------------------+-------------+\n" );
  oss . str ("");
  oss . clear ();
  oss << t1;
  assert ( oss . str () ==
        "+----------------------------------------------+------------------------------------------+\n"
        "|+------------+-------------+                  |          ###                             |\n"
        "||OOP         |Encapsulation|                  |          #  #                            |\n"
        "|+------------+-------------+                  |          #  # # ##   ###    ###          |\n"
        "||Polymorphism|Inheritance  |                  |          ###  ##    #   #  #  #          |\n"
        "|+------------+-------------+                  |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #     #   #  #  #          |\n"
        "|                                              |          #    #      ###    ###          |\n"
        "|                                              |                               #          |\n"
        "|                                              |                             ##           |\n"
        "|                                              |                                          |\n"
        "|                                              |           #    ###   ###   #             |\n"
        "|                                              |          ###  #   # #     ###            |\n"
        "|                                              |           #   #####  ###   #             |\n"
        "|                                              |           #   #         #  #             |\n"
        "|                                              |            ##  ###   ###    ##           |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|Lorem ipsum dolor sit amet,                   |                                          |\n"
        "|consectetur adipiscing                        |                                          |\n"
        "|elit. Curabitur scelerisque                   |        ********                          |\n"
        "|lorem vitae lectus cursus,                    |       **********                         |\n"
        "|vitae porta ante placerat. Class aptent taciti|      **        **                        |\n"
        "|sociosqu ad litora                            |      **             **        **         |\n"
        "|torquent per                                  |      **             **        **         |\n"
        "|conubia nostra,                               |      ***         ********  ********      |\n"
        "|per inceptos himenaeos.                       |      ****        ********  ********      |\n"
        "|                                              |      ****           **        **         |\n"
        "|Donec tincidunt augue                         |      ****           **        **         |\n"
        "|sit amet metus                                |      ****      **                        |\n"
        "|pretium volutpat.                             |       **********                         |\n"
        "|Donec faucibus,                               |        ********                          |\n"
        "|ante sit amet                                 |                                          |\n"
        "|luctus posuere,                               |                                          |\n"
        "|mauris tellus                                 |                                          |\n"
        "+----------------------------------------------+------------------------------------------+\n"
        "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
        "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|                                              |*    *  *      *  *      *           *   *|\n"
        "|                                              |*    *  *      *  *      *****      *    *|\n"
        "|                                              |****    *      *  *      *         *     *|\n"
        "|                                              |*  *    *      *  *      *        *       |\n"
        "|                                              |*   *   *      *  *      *       *       *|\n"
        "|                                              |*    *    *****   ****** ******* ******  *|\n"
        "+----------------------------------------------+------------------------------------------+\n" );
  assert ( t0 != t1 );
  assert ( !( t0 == t1 ) );
  assert ( t0 . getCell ( 1, 1 ) == t0 . getCell ( 0, 1 ) );
  assert ( ! ( t0 . getCell ( 1, 1 ) != t0 . getCell ( 0, 1 ) ) );
  assert ( t0 . getCell ( 0, 0 ) != t0 . getCell ( 0, 1 ) );
  assert ( ! ( t0 . getCell ( 0, 0 ) == t0 . getCell ( 0, 1 ) ) );
  t1 . setCell ( 0, 0, t1 );
  oss . str ("");
  oss . clear ();
  oss << t1;
  assert ( oss . str () ==
        "+-------------------------------------------------------------------------------------------+------------------------------------------+\n"
        "|+----------------------------------------------+------------------------------------------+|                                          |\n"
        "||+------------+-------------+                  |          ###                             ||                                          |\n"
        "|||OOP         |Encapsulation|                  |          #  #                            ||                                          |\n"
        "||+------------+-------------+                  |          #  # # ##   ###    ###          ||                                          |\n"
        "|||Polymorphism|Inheritance  |                  |          ###  ##    #   #  #  #          ||                                          |\n"
        "||+------------+-------------+                  |          #    #     #   #  #  #          ||                                          |\n"
        "||                                              |          #    #     #   #  #  #          ||                                          |\n"
        "||                                              |          #    #      ###    ###          ||                                          |\n"
        "||                                              |                               #          ||                                          |\n"
        "||                                              |                             ##           ||                                          |\n"
        "||                                              |                                          ||                                          |\n"
        "||                                              |           #    ###   ###   #             ||                                          |\n"
        "||                                              |          ###  #   # #     ###            ||                                          |\n"
        "||                                              |           #   #####  ###   #             ||                                          |\n"
        "||                                              |           #   #         #  #             ||          ###                             |\n"
        "||                                              |            ##  ###   ###    ##           ||          #  #                            |\n"
        "|+----------------------------------------------+------------------------------------------+|          #  # # ##   ###    ###          |\n"
        "||Lorem ipsum dolor sit amet,                   |                                          ||          ###  ##    #   #  #  #          |\n"
        "||consectetur adipiscing                        |                                          ||          #    #     #   #  #  #          |\n"
        "||elit. Curabitur scelerisque                   |        ********                          ||          #    #     #   #  #  #          |\n"
        "||lorem vitae lectus cursus,                    |       **********                         ||          #    #      ###    ###          |\n"
        "||vitae porta ante placerat. Class aptent taciti|      **        **                        ||                               #          |\n"
        "||sociosqu ad litora                            |      **             **        **         ||                             ##           |\n"
        "||torquent per                                  |      **             **        **         ||                                          |\n"
        "||conubia nostra,                               |      ***         ********  ********      ||           #    ###   ###   #             |\n"
        "||per inceptos himenaeos.                       |      ****        ********  ********      ||          ###  #   # #     ###            |\n"
        "||                                              |      ****           **        **         ||           #   #####  ###   #             |\n"
        "||Donec tincidunt augue                         |      ****           **        **         ||           #   #         #  #             |\n"
        "||sit amet metus                                |      ****      **                        ||            ##  ###   ###    ##           |\n"
        "||pretium volutpat.                             |       **********                         ||                                          |\n"
        "||Donec faucibus,                               |        ********                          ||                                          |\n"
        "||ante sit amet                                 |                                          ||                                          |\n"
        "||luctus posuere,                               |                                          ||                                          |\n"
        "||mauris tellus                                 |                                          ||                                          |\n"
        "|+----------------------------------------------+------------------------------------------+|                                          |\n"
        "||                                          Bye,|*****   *      *  *      ******* ******  *||                                          |\n"
        "||                                   Hello Kitty|*    *  *      *  *      *            *  *||                                          |\n"
        "||                                              |*    *  *      *  *      *           *   *||                                          |\n"
        "||                                              |*    *  *      *  *      *****      *    *||                                          |\n"
        "||                                              |****    *      *  *      *         *     *||                                          |\n"
        "||                                              |*  *    *      *  *      *        *       ||                                          |\n"
        "||                                              |*   *   *      *  *      *       *       *||                                          |\n"
        "||                                              |*    *    *****   ****** ******* ******  *||                                          |\n"
        "|+----------------------------------------------+------------------------------------------+|                                          |\n"
        "+-------------------------------------------------------------------------------------------+------------------------------------------+\n"
        "|Lorem ipsum dolor sit amet,                                                                |                                          |\n"
        "|consectetur adipiscing                                                                     |                                          |\n"
        "|elit. Curabitur scelerisque                                                                |        ********                          |\n"
        "|lorem vitae lectus cursus,                                                                 |       **********                         |\n"
        "|vitae porta ante placerat. Class aptent taciti                                             |      **        **                        |\n"
        "|sociosqu ad litora                                                                         |      **             **        **         |\n"
        "|torquent per                                                                               |      **             **        **         |\n"
        "|conubia nostra,                                                                            |      ***         ********  ********      |\n"
        "|per inceptos himenaeos.                                                                    |      ****        ********  ********      |\n"
        "|                                                                                           |      ****           **        **         |\n"
        "|Donec tincidunt augue                                                                      |      ****           **        **         |\n"
        "|sit amet metus                                                                             |      ****      **                        |\n"
        "|pretium volutpat.                                                                          |       **********                         |\n"
        "|Donec faucibus,                                                                            |        ********                          |\n"
        "|ante sit amet                                                                              |                                          |\n"
        "|luctus posuere,                                                                            |                                          |\n"
        "|mauris tellus                                                                              |                                          |\n"
        "+-------------------------------------------------------------------------------------------+------------------------------------------+\n"
        "|                                                                                       Bye,|*****   *      *  *      ******* ******  *|\n"
        "|                                                                                Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|                                                                                           |*    *  *      *  *      *           *   *|\n"
        "|                                                                                           |*    *  *      *  *      *****      *    *|\n"
        "|                                                                                           |****    *      *  *      *         *     *|\n"
        "|                                                                                           |*  *    *      *  *      *        *       |\n"
        "|                                                                                           |*   *   *      *  *      *       *       *|\n"
        "|                                                                                           |*    *    *****   ****** ******* ******  *|\n"
        "+-------------------------------------------------------------------------------------------+------------------------------------------+\n" );
  t1 . setCell ( 0, 0, t1 );
  oss . str ("");
  oss . clear ();
  oss << t1;
  assert ( oss . str () ==
        "+----------------------------------------------------------------------------------------------------------------------------------------+------------------------------------------+\n"
        "|+-------------------------------------------------------------------------------------------+------------------------------------------+|                                          |\n"
        "||+----------------------------------------------+------------------------------------------+|                                          ||                                          |\n"
        "|||+------------+-------------+                  |          ###                             ||                                          ||                                          |\n"
        "||||OOP         |Encapsulation|                  |          #  #                            ||                                          ||                                          |\n"
        "|||+------------+-------------+                  |          #  # # ##   ###    ###          ||                                          ||                                          |\n"
        "||||Polymorphism|Inheritance  |                  |          ###  ##    #   #  #  #          ||                                          ||                                          |\n"
        "|||+------------+-------------+                  |          #    #     #   #  #  #          ||                                          ||                                          |\n"
        "|||                                              |          #    #     #   #  #  #          ||                                          ||                                          |\n"
        "|||                                              |          #    #      ###    ###          ||                                          ||                                          |\n"
        "|||                                              |                               #          ||                                          ||                                          |\n"
        "|||                                              |                             ##           ||                                          ||                                          |\n"
        "|||                                              |                                          ||                                          ||                                          |\n"
        "|||                                              |           #    ###   ###   #             ||                                          ||                                          |\n"
        "|||                                              |          ###  #   # #     ###            ||                                          ||                                          |\n"
        "|||                                              |           #   #####  ###   #             ||                                          ||                                          |\n"
        "|||                                              |           #   #         #  #             ||          ###                             ||                                          |\n"
        "|||                                              |            ##  ###   ###    ##           ||          #  #                            ||                                          |\n"
        "||+----------------------------------------------+------------------------------------------+|          #  # # ##   ###    ###          ||                                          |\n"
        "|||Lorem ipsum dolor sit amet,                   |                                          ||          ###  ##    #   #  #  #          ||                                          |\n"
        "|||consectetur adipiscing                        |                                          ||          #    #     #   #  #  #          ||                                          |\n"
        "|||elit. Curabitur scelerisque                   |        ********                          ||          #    #     #   #  #  #          ||                                          |\n"
        "|||lorem vitae lectus cursus,                    |       **********                         ||          #    #      ###    ###          ||                                          |\n"
        "|||vitae porta ante placerat. Class aptent taciti|      **        **                        ||                               #          ||                                          |\n"
        "|||sociosqu ad litora                            |      **             **        **         ||                             ##           ||                                          |\n"
        "|||torquent per                                  |      **             **        **         ||                                          ||                                          |\n"
        "|||conubia nostra,                               |      ***         ********  ********      ||           #    ###   ###   #             ||                                          |\n"
        "|||per inceptos himenaeos.                       |      ****        ********  ********      ||          ###  #   # #     ###            ||                                          |\n"
        "|||                                              |      ****           **        **         ||           #   #####  ###   #             ||                                          |\n"
        "|||Donec tincidunt augue                         |      ****           **        **         ||           #   #         #  #             ||                                          |\n"
        "|||sit amet metus                                |      ****      **                        ||            ##  ###   ###    ##           ||          ###                             |\n"
        "|||pretium volutpat.                             |       **********                         ||                                          ||          #  #                            |\n"
        "|||Donec faucibus,                               |        ********                          ||                                          ||          #  # # ##   ###    ###          |\n"
        "|||ante sit amet                                 |                                          ||                                          ||          ###  ##    #   #  #  #          |\n"
        "|||luctus posuere,                               |                                          ||                                          ||          #    #     #   #  #  #          |\n"
        "|||mauris tellus                                 |                                          ||                                          ||          #    #     #   #  #  #          |\n"
        "||+----------------------------------------------+------------------------------------------+|                                          ||          #    #      ###    ###          |\n"
        "|||                                          Bye,|*****   *      *  *      ******* ******  *||                                          ||                               #          |\n"
        "|||                                   Hello Kitty|*    *  *      *  *      *            *  *||                                          ||                             ##           |\n"
        "|||                                              |*    *  *      *  *      *           *   *||                                          ||                                          |\n"
        "|||                                              |*    *  *      *  *      *****      *    *||                                          ||           #    ###   ###   #             |\n"
        "|||                                              |****    *      *  *      *         *     *||                                          ||          ###  #   # #     ###            |\n"
        "|||                                              |*  *    *      *  *      *        *       ||                                          ||           #   #####  ###   #             |\n"
        "|||                                              |*   *   *      *  *      *       *       *||                                          ||           #   #         #  #             |\n"
        "|||                                              |*    *    *****   ****** ******* ******  *||                                          ||            ##  ###   ###    ##           |\n"
        "||+----------------------------------------------+------------------------------------------+|                                          ||                                          |\n"
        "|+-------------------------------------------------------------------------------------------+------------------------------------------+|                                          |\n"
        "||Lorem ipsum dolor sit amet,                                                                |                                          ||                                          |\n"
        "||consectetur adipiscing                                                                     |                                          ||                                          |\n"
        "||elit. Curabitur scelerisque                                                                |        ********                          ||                                          |\n"
        "||lorem vitae lectus cursus,                                                                 |       **********                         ||                                          |\n"
        "||vitae porta ante placerat. Class aptent taciti                                             |      **        **                        ||                                          |\n"
        "||sociosqu ad litora                                                                         |      **             **        **         ||                                          |\n"
        "||torquent per                                                                               |      **             **        **         ||                                          |\n"
        "||conubia nostra,                                                                            |      ***         ********  ********      ||                                          |\n"
        "||per inceptos himenaeos.                                                                    |      ****        ********  ********      ||                                          |\n"
        "||                                                                                           |      ****           **        **         ||                                          |\n"
        "||Donec tincidunt augue                                                                      |      ****           **        **         ||                                          |\n"
        "||sit amet metus                                                                             |      ****      **                        ||                                          |\n"
        "||pretium volutpat.                                                                          |       **********                         ||                                          |\n"
        "||Donec faucibus,                                                                            |        ********                          ||                                          |\n"
        "||ante sit amet                                                                              |                                          ||                                          |\n"
        "||luctus posuere,                                                                            |                                          ||                                          |\n"
        "||mauris tellus                                                                              |                                          ||                                          |\n"
        "|+-------------------------------------------------------------------------------------------+------------------------------------------+|                                          |\n"
        "||                                                                                       Bye,|*****   *      *  *      ******* ******  *||                                          |\n"
        "||                                                                                Hello Kitty|*    *  *      *  *      *            *  *||                                          |\n"
        "||                                                                                           |*    *  *      *  *      *           *   *||                                          |\n"
        "||                                                                                           |*    *  *      *  *      *****      *    *||                                          |\n"
        "||                                                                                           |****    *      *  *      *         *     *||                                          |\n"
        "||                                                                                           |*  *    *      *  *      *        *       ||                                          |\n"
        "||                                                                                           |*   *   *      *  *      *       *       *||                                          |\n"
        "||                                                                                           |*    *    *****   ****** ******* ******  *||                                          |\n"
        "|+-------------------------------------------------------------------------------------------+------------------------------------------+|                                          |\n"
        "+----------------------------------------------------------------------------------------------------------------------------------------+------------------------------------------+\n"
        "|Lorem ipsum dolor sit amet,                                                                                                             |                                          |\n"
        "|consectetur adipiscing                                                                                                                  |                                          |\n"
        "|elit. Curabitur scelerisque                                                                                                             |        ********                          |\n"
        "|lorem vitae lectus cursus,                                                                                                              |       **********                         |\n"
        "|vitae porta ante placerat. Class aptent taciti                                                                                          |      **        **                        |\n"
        "|sociosqu ad litora                                                                                                                      |      **             **        **         |\n"
        "|torquent per                                                                                                                            |      **             **        **         |\n"
        "|conubia nostra,                                                                                                                         |      ***         ********  ********      |\n"
        "|per inceptos himenaeos.                                                                                                                 |      ****        ********  ********      |\n"
        "|                                                                                                                                        |      ****           **        **         |\n"
        "|Donec tincidunt augue                                                                                                                   |      ****           **        **         |\n"
        "|sit amet metus                                                                                                                          |      ****      **                        |\n"
        "|pretium volutpat.                                                                                                                       |       **********                         |\n"
        "|Donec faucibus,                                                                                                                         |        ********                          |\n"
        "|ante sit amet                                                                                                                           |                                          |\n"
        "|luctus posuere,                                                                                                                         |                                          |\n"
        "|mauris tellus                                                                                                                           |                                          |\n"
        "+----------------------------------------------------------------------------------------------------------------------------------------+------------------------------------------+\n"
        "|                                                                                                                                    Bye,|*****   *      *  *      ******* ******  *|\n"
        "|                                                                                                                             Hello Kitty|*    *  *      *  *      *            *  *|\n"
        "|                                                                                                                                        |*    *  *      *  *      *           *   *|\n"
        "|                                                                                                                                        |*    *  *      *  *      *****      *    *|\n"
        "|                                                                                                                                        |****    *      *  *      *         *     *|\n"
        "|                                                                                                                                        |*  *    *      *  *      *        *       |\n"
        "|                                                                                                                                        |*   *   *      *  *      *       *       *|\n"
        "|                                                                                                                                        |*    *    *****   ****** ******* ******  *|\n"
        "+----------------------------------------------------------------------------------------------------------------------------------------+------------------------------------------+\n" );

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
