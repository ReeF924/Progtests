#include <cstring>
#ifndef __PROGTEST__
#include <cassert>
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
#include <compare>
#include <functional>
#include <optional>

class CTimeStamp
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    double second;

public:
    CTimeStamp(int year, int month, int day, int hour, int minute, double sec)
        : year(year), month(month), day(day), hour(hour), minute(minute), second(sec)
    {
    }

    int compare(const CTimeStamp& x) const
    {
        //a = this, b = x
        //a > b +
        //a == b 0
        //a < b -

        int res = year - x.year;
        if (res != 0) return res;

        res = month - x.month;
        if (res != 0) return res;

        res = day - x.day;
        if (res != 0) return res;

        res = hour - x.hour;
        if (res != 0) return res;

        res = minute - x.minute;
        if (res != 0) return res;

        const double d = second - x.second;
        if (d > 0) return 1;
        if (d < 0) return -1;
        return 0;
    }

    friend std::ostream& operator <<(std::ostream& os, const CTimeStamp& x)
    {
        return os << std::setw(4) << std::setfill('0') << x.year
            << '-' << std::setw(2) << std::setfill('0') << x.month
            << '-' << std::setw(2) << std::setfill('0') << x.day
            << ' ' << std::setw(2) << std::setfill('0') << x.hour
            << ':' << std::setw(2) << std::setfill('0') << x.minute
            << ':' << std::setw(6) << std::setfill('0') << std::fixed << std::setprecision(3) << x.second;
    }

private:
};

class CMail
{
    CTimeStamp _timeStamp;
    std::string _from;
    std::string _to;
    std::optional<std::string> _subject;

public:
    CMail(const CTimeStamp& timeStamp, const std::string& from, const std::string& to,
          const std::optional<std::string>& subject)
        : _timeStamp(timeStamp), _from(from), _to(to), _subject(subject)
    {
    }

    int compareByTime(const CTimeStamp& x) const
    {
        return _timeStamp.compare(x);
    }

    int compareByTime(const CMail& x) const
    {
        return _timeStamp.compare(x._timeStamp);
    }

    const std::string& from() const
    {
        return _from;
    }

    const std::string& to() const
    {
        return _to;
    }

    const std::optional<std::string>& subject() const
    {
        return _subject;
    }

    const CTimeStamp& timeStamp() const
    {
        return _timeStamp;
    }

    friend std::ostream& operator <<(std::ostream& os, const CMail& x)
    {
        //format:
        // 2025-03-29 14:58:32.000 PR-department@fit.cvut.cz -> HR-department@fit.cvut.cz, subject: Business partner
        if (x._subject)
        {
            return os << x._timeStamp << " " << x._from << " -> " << x._to << ", subject: " << x._subject.value().c_str();
        }

        return os << x._timeStamp << " " << x._from << " -> " << x._to;
    }

private:
};

// your code will be compiled in a separate namespace
namespace MysteriousNamespace
{
#endif /* __PROGTEST__ */
    //----------------------------------------------------------------------------------------

    class MailHeader
    {
    public:
        std::string from;
        std::optional<std::string> subject;
        std::vector<std::pair<std::string, CTimeStamp>> headerMails;

        explicit MailHeader(std::string from) noexcept
            : from(std::move(from)), subject({}), headerMails({})
        {
        }

        ~MailHeader() = default;
    };


    class CMailLog
    {
        std::vector<CMail> mails;

    public:
        int parseLog(std::istream& in)
        {
            //format
            //"Mar 29 2025 12:35:32.233 relay.fit.cvut.cz ADFger72343D: from=user1@fit.cvut.cz\n"

            std::map<std::string, MailHeader> headers;

            int counter = 0;
            while (!in.eof())
            {
                char mon[4];
                int day, year, hour, min;
                double sec;
                char c1, c2;

                if (!(in >> mon >> day >> year >> hour >> c1 >> min >> c2 >> sec))
                {
                    std::string dumpedLine;
                    std::getline(in, dumpedLine);
                    continue;
                }

                CTimeStamp tms(year, monthToNumber(mon), day, hour, min, sec);


                //"Mar 29 2025 12:35:32.233 relay.fit.cvut.cz ADFger72343D: from=user1@fit.cvut.cz\n"
                std::string from;

                //dumping the server
                in >> from;

                std::string id, description;

                in >> id >> description;


                if (startsWith(description, "from="))
                {
                    headers.insert(std::pair(id, MailHeader(description.substr(5))));

                }
                else if (startsWith(description, "to="))
                {
                    auto found = std::upper_bound(mails.begin(), mails.end(), tms, compareCTimeStampUpper);
                    MailHeader& header = headers.at(id);

                    mails.emplace(found, tms, header.from, description.substr(3), header.subject);
                    ++counter;
                }
                else if (startsWith(description, "subject="))
                {
                    MailHeader& header = headers.at(id);
                    std::string line;
                    std::getline(in, line);
                    header.subject = description.substr(8) + line;
                }
                else
                {
                    std::string dumpedLine;
                    std::getline(in, dumpedLine);
                }
            }

            return counter;
        }

        std::list<CMail> listMail(const CTimeStamp& from, const CTimeStamp& to) const
        {
            std::list<CMail> list = {};

            if (from.compare(to) > 0)
            {
                return list;
            }

            auto beginIt = std::lower_bound(mails.begin(), mails.end(), from, compareCTimeStampLower);

            if (beginIt == mails.end())
            {
                return list;
            }

            const auto endIt = std::upper_bound(mails.begin(), mails.end(), to, compareCTimeStampUpper);

            for (; beginIt != endIt; ++beginIt)
            {
                list.push_back(*beginIt);
            }

            return list;
        }

        std::set<std::string> activeUsers(const CTimeStamp& from, const CTimeStamp& to) const
        {
            std::set<std::string> out = {};

            if (from.compare(to) > 0)
            {
                return out;
            }

            auto beginIt = std::lower_bound(mails.begin(), mails.end(), from, compareCTimeStampLower);

            if (beginIt == mails.end())
            {
                return out;
            }

            const auto endIt = std::upper_bound(mails.begin(), mails.end(), to, compareCTimeStampUpper);

            for (; beginIt != endIt; ++beginIt)
            {
                out.insert(beginIt->from());
                out.insert(beginIt->to());
            }

            return out;
        }

    private:
        static int monthToNumber(const char month[4])
        {
            if (strcmp(month, "Jan") == 0) return 1;
            if (strcmp(month, "Feb") == 0) return 2;
            if (strcmp(month, "Mar") == 0) return 3;
            if (strcmp(month, "Apr") == 0) return 4;
            if (strcmp(month, "May") == 0) return 5;
            if (strcmp(month, "Jun") == 0) return 6;
            if (strcmp(month, "Jul") == 0) return 7;
            if (strcmp(month, "Aug") == 0) return 8;
            if (strcmp(month, "Sep") == 0) return 9;
            if (strcmp(month, "Oct") == 0) return 10;
            if (strcmp(month, "Nov") == 0) return 11;
            if (strcmp(month, "Dec") == 0) return 12;

            throw std::invalid_argument("Invalid month value");
        }

        static bool startsWith(const std::string& s, const char* start)
        {
            const size_t len = strlen(start);

            if (len > s.length()) return false;

            for (size_t i = 0; i < len; ++i)
            {
                if (s[i] != start[i])
                {
                    return false;
                }
            }
            return true;
        }

        static bool compareCTimeStampLower(const CMail& a, const CTimeStamp& b)
        {
            return a.compareByTime(b) < 0;
        }
        static bool compareCTimeStampUpper(const CTimeStamp& a, const CMail& b)
        {
            return b.compareByTime(a) > 0;
        }
    };

    //----------------------------------------------------------------------------------------
#ifndef __PROGTEST__
} // namespace
std::string printMail(const std::list<CMail>& all)
{
    std::ostringstream oss;
    for (const auto& mail : all)
        oss << mail << "\n";
    return oss.str();
}

int main()
{
    CTimeStamp tm(2012, 10, 1, 4, 69, 86.4234);
    std::stringstream oss;

    oss << tm;

    assert(oss.str() == "2012-10-01 04:69:86.423");


    CTimeStamp a(4, 4, 2025, 14, 30, 45.123);
    CTimeStamp b(4, 4, 2025, 14, 30, 45.123);
    CTimeStamp c(4, 4, 2026, 14, 30, 45.123);
    CTimeStamp d(4, 5, 2025, 14, 30, 45.123);
    CTimeStamp e(5, 4, 2025, 14, 30, 45.123);
    CTimeStamp f(4, 4, 2025, 15, 30, 45.123);
    CTimeStamp g(4, 4, 2025, 14, 31, 45.123);
    CTimeStamp h(4, 4, 2025, 14, 30, 46.123);


    assert(a.compare(b) == 0);

    assert(a.compare(c) < 0);
    assert(c.compare(a) > 0);

    assert(a.compare(d) < 0);
    assert(d.compare(a) > 0);

    assert(a.compare(e) < 0);
    assert(e.compare(a) > 0);

    assert(a.compare(f) < 0);
    assert(f.compare(a) > 0);

    assert(a.compare(g) < 0);
    assert(g.compare(a) > 0);

    assert(a.compare(h) < 0);
    assert(h.compare(a) > 0);


    MysteriousNamespace::CMailLog m;
    std::list<CMail> mailList;
    std::set<std::string> users;
    std::istringstream iss;

    CTimeStamp tm1(2025, 3,29,12,35,32.231);
    CTimeStamp tm2(2025, 3,29,12,35,32.230);

    assert(tm1.compare(tm2) > 0);

        iss.clear();
    iss.str(
        "Mar 29 2025 12:35:32.233 relay.fit.cvut.cz ADFger72343D: from=user1@fit.cvut.cz\n"
        "Mar 29 2025 12:37:16.234 relay.fit.cvut.cz JlMSRW4232Df: from=person3@fit.cvut.cz\n"
        "Mar 29 2025 12:55:13.023 relay.fit.cvut.cz JlMSRW4232Df: subject=New progtest homework!\n"
        "Mar 29 2025 13:38:45.043 relay.fit.cvut.cz Kbced342sdgA: from=office13@fit.cvut.cz\n"
        "Mar 29 2025 13:36:13.023 relay.fit.cvut.cz JlMSRW4232Df: to=user76@fit.cvut.cz\n"
        "Mar 29 2025 13:55:31.456 relay.fit.cvut.cz KhdfEjkl247D: from=PR-department@fit.cvut.cz\n"
        "Mar 29 2025 14:18:12.654 relay.fit.cvut.cz Kbced342sdgA: to=boss13@fit.cvut.cz\n"
        "Mar 29 2025 14:48:32.563 relay.fit.cvut.cz KhdfEjkl247D: subject=Business partner\n"
        "Mar 29 2025 14:25:23.233 relay.fit.cvut.cz ADFger72343D: mail undeliverable\n");

    assert(m . parseLog ( iss ) == 2);
    mailList = m.listMail(CTimeStamp(2025, 3, 28, 0, 0, 0),
                          CTimeStamp(2025, 3, 29, 23, 59, 59));

    std::string s = printMail(mailList);
    assert(
        printMail ( mailList ) ==
R"###(2025-03-29 13:36:13.023 person3@fit.cvut.cz -> user76@fit.cvut.cz, subject: New progtest homework!
2025-03-29 14:18:12.654 office13@fit.cvut.cz -> boss13@fit.cvut.cz
)###");

    m = MysteriousNamespace::CMailLog();
    iss.clear();
    iss.str(
        "Mar 29 2025 12:35:32.233 relay.fit.cvut.cz ADFger72343D: from=user1@fit.cvut.cz\n"
        "Mar 29 2025 12:37:16.234 relay.fit.cvut.cz JlMSRW4232Df: from=person3@fit.cvut.cz\n"
        "Mar 29 2025 12:55:13.023 relay.fit.cvut.cz JlMSRW4232Df: subject=New progtest homework!\n"
        "Mar 29 2025 13:38:45.043 relay.fit.cvut.cz Kbced342sdgA: from=office13@fit.cvut.cz\n"
        "Mar 29 2025 13:36:13.023 relay.fit.cvut.cz JlMSRW4232Df: to=user76@fit.cvut.cz\n"
        "Mar 29 2025 13:55:31.456 relay.fit.cvut.cz KhdfEjkl247D: from=PR-department@fit.cvut.cz\n"
        "Mar 29 2025 14:18:12.654 relay.fit.cvut.cz Kbced342sdgA: to=boss13@fit.cvut.cz\n"
        "Mar 29 2025 14:48:32.563 relay.fit.cvut.cz KhdfEjkl247D: subject=Business partner\n"
        "Mar 29 2025 14:58:32.000 relay.fit.cvut.cz KhdfEjkl247D: to=HR-department@fit.cvut.cz\n"
        "Mar 29 2025 14:25:23.233 relay.fit.cvut.cz ADFger72343D: mail undeliverable\n"
        "Mar 29 2025 15:02:34.231 relay.fit.cvut.cz KhdfEjkl247D: to=CIO@fit.cvut.cz\n"
        "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=CEO@fit.cvut.cz\n"
        "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=dean@fit.cvut.cz\n"
        "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=vice-dean@fit.cvut.cz\n"
        "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=archive@fit.cvut.cz\n");
    assert(m . parseLog ( iss ) == 8);
    mailList = m.listMail(CTimeStamp(2025, 3, 28, 0, 0, 0),
                          CTimeStamp(2025, 3, 29, 23, 59, 59));

    s = printMail(mailList);
    assert(
        printMail ( mailList ) ==
        R"###(2025-03-29 13:36:13.023 person3@fit.cvut.cz -> user76@fit.cvut.cz, subject: New progtest homework!
2025-03-29 14:18:12.654 office13@fit.cvut.cz -> boss13@fit.cvut.cz
2025-03-29 14:58:32.000 PR-department@fit.cvut.cz -> HR-department@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> CEO@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> dean@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> vice-dean@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> archive@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.231 PR-department@fit.cvut.cz -> CIO@fit.cvut.cz, subject: Business partner
)###");
    mailList = m.listMail(CTimeStamp(2025, 3, 28, 0, 0, 0),
                          CTimeStamp(2025, 3, 29, 14, 58, 32));
    assert(
        printMail ( mailList ) ==
R"###(2025-03-29 13:36:13.023 person3@fit.cvut.cz -> user76@fit.cvut.cz, subject: New progtest homework!
2025-03-29 14:18:12.654 office13@fit.cvut.cz -> boss13@fit.cvut.cz
2025-03-29 14:58:32.000 PR-department@fit.cvut.cz -> HR-department@fit.cvut.cz, subject: Business partner
)###");
    mailList = m.listMail(CTimeStamp(2025, 3, 30, 0, 0, 0),
                          CTimeStamp(2025, 3, 30, 23, 59, 59));
    assert(printMail ( mailList ) == "");


    users = m.activeUsers(CTimeStamp(2025, 3, 28, 0, 0, 0),
                          CTimeStamp(2025, 3, 29, 23, 59, 59));
    assert(
        users == std::set<std::string>( { "CEO@fit.cvut.cz", "CIO@fit.cvut.cz", "HR-department@fit.cvut.cz",
            "PR-department@fit.cvut.cz", "archive@fit.cvut.cz", "boss13@fit.cvut.cz", "dean@fit.cvut.cz",
            "office13@fit.cvut.cz", "person3@fit.cvut.cz", "user76@fit.cvut.cz", "vice-dean@fit.cvut.cz" } ));
    users = m.activeUsers(CTimeStamp(2025, 3, 28, 0, 0, 0),
                          CTimeStamp(2025, 3, 29, 13, 59, 59));
    assert(users == std::set<std::string>( { "person3@fit.cvut.cz", "user76@fit.cvut.cz" } ));


    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
