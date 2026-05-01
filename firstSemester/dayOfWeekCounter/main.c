#include <math.h>
#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

constexpr unsigned DOW_MON = 0b0000'0001;
constexpr unsigned DOW_TUE = 0b0000'0010;
constexpr unsigned DOW_WED = 0b0000'0100;
constexpr unsigned DOW_THU = 0b0000'1000;
constexpr unsigned DOW_FRI = 0b0001'0000;
constexpr unsigned DOW_SAT = 0b0010'0000;
constexpr unsigned DOW_SUN = 0b0100'0000;
constexpr unsigned DOW_WORKDAYS = DOW_MON | DOW_TUE | DOW_WED | DOW_THU | DOW_FRI;
constexpr unsigned DOW_WEEKEND = DOW_SAT | DOW_SUN;
constexpr unsigned DOW_ALL = DOW_WORKDAYS | DOW_WEEKEND;


typedef struct TDate {
    unsigned m_Year;
    unsigned m_Month;
    unsigned m_Day;
} TDATE;


TDATE makeDate(unsigned y, unsigned m, unsigned d) {
    TDATE res = {y, m, d};
    return res;
}
#endif /* __PROGTEST__ */

bool isValidDateSequence(TDATE first, TDATE second);

bool isValidDate(TDATE date);

bool IsLeapYear(unsigned year);

unsigned char GetDayOfWeek(TDATE date);

unsigned int GetConnectionsPerWeek(unsigned perWorkDay, unsigned dowMask, unsigned char startDow);

unsigned int GetConnectionsPerWeekInverted(unsigned perWorkDay, unsigned dowMask, unsigned char endDow);

long long GetDayDifference(TDATE from, TDATE to);

long long GetTotalDaysInDate(TDATE date);

char GetLastPossibleDayInWeek(long long connections, unsigned perWorkDay, unsigned dowMask);

unsigned int GetConnectionsForDays(long long days, unsigned perWorkDay, unsigned dowMask, unsigned char startDow);

TDATE IncrementDateByDays(TDATE date, long long days);

long long GetDaysWhenConnectionsLessThanWeek(long long connections, unsigned perWorkDay, unsigned dowMask,
                                             unsigned char startDow);

long long countConnections(TDATE from, TDATE to, unsigned perWorkDay, unsigned dowMask) {
    if (!isValidDate(from) || !isValidDate(to)) {
        return -1;
    }
    if (!isValidDateSequence(from, to)) {
        return -1;
    }

    const unsigned char dowFrom = GetDayOfWeek(from);
    const unsigned char dowTo = GetDayOfWeek(to);

    long long days = GetDayDifference(from, to);

    const unsigned connectionsPerWeek = GetConnectionsPerWeek(perWorkDay, dowMask, 0);

    long long connectionsCount = 0;

    if (days < 7 - dowFrom) {
        return GetConnectionsForDays(days, perWorkDay, dowMask, dowFrom);
    }

    if (dowFrom != 0) {
        connectionsCount += GetConnectionsPerWeek(perWorkDay, dowMask, dowFrom);
        days -= 7 - dowFrom;
    }

    if (dowTo != 6) {
        connectionsCount += GetConnectionsPerWeekInverted(perWorkDay, dowMask, dowTo);
        days -= dowTo + 1;
    }

    connectionsCount += (days / 7) * connectionsPerWeek;

    return connectionsCount;
}

//returns the last date when all connections can be made (so if I need 2 for next day (tuesday for instance) and only have 1 I return this day(monday))
TDATE endDate(TDATE from, long long connections, unsigned perWorkDay, unsigned dowMask) {
    //todo if the connection won't last a dat return 0000-00-00

    if (perWorkDay == 0 || dowMask == 0 || !isValidDate(from) || connections < 0) {
        return makeDate(0, 0, 0);
    }

    const unsigned char dowFrom = GetDayOfWeek(from);

    long long days = 0;

    const unsigned connectionsPerWeek = GetConnectionsPerWeek(perWorkDay, dowMask, 0);

    if (connectionsPerWeek > connections) {
        days = GetDaysWhenConnectionsLessThanWeek(connections, perWorkDay, dowMask, dowFrom);
        if (days == 0) {
            return makeDate(0, 0, 0);
        }
        return IncrementDateByDays(from, days - 1);
    }


    if (dowFrom != 0) {
        connections -= GetConnectionsPerWeek(perWorkDay, dowMask, dowFrom);
        days += 7 - dowFrom;
    }

    const unsigned weeksAvailable = connections / connectionsPerWeek;
    days += weeksAvailable * 7;
    connections -= connectionsPerWeek * weeksAvailable;


    // const TDATE dateBeforeEndDow = IncrementDateByDays(from, days);

    //shouldn't be more than 6 iterations
    char endDow = GetLastPossibleDayInWeek(connections, perWorkDay, dowMask);

    days += endDow + 1;

    return IncrementDateByDays(from, days - 1);
}
#ifndef __PROGTEST__
int main() {
    //my own asserts

    TDATE d;


    //friday
    d = endDate(makeDate(4001, 1, 13), 5, 6, DOW_WEEKEND);
    assert(d.m_Year == 4001 && d.m_Month == 1 && d.m_Day == 20);
    d = endDate(makeDate(4001, 1, 13), 5, 10, DOW_WEEKEND);
    assert(d.m_Year == 4001 && d.m_Month == 1 && d.m_Day == 14);
    d = endDate(makeDate(4001, 1, 13), 5, 15, DOW_WEEKEND);
    assert(d.m_Year == 4001 && d.m_Month == 1 && d.m_Day == 13);


    //progtestIssues
    d = endDate(makeDate(4001, 1, 13), 5, 6, DOW_WED);
    assert(d.m_Year == 4001 && d.m_Month == 1 && d.m_Day == 17);
    d = endDate(makeDate(2004, 1, 30), 3271, 16, DOW_TUE | DOW_WED | DOW_SUN);
    assert(d.m_Year == 2005 && d.m_Month == 9 && d.m_Day == 24);
    // endDate ( makeDate ( 4001, 1, 1 ), 5, 6, DOW_MON ) => r={4001, 1, 1}, s={4001, 1, 2}
    d = endDate ( makeDate ( 4001, 1, 1 ), 5, 6, DOW_MON );






    d = endDate(makeDate(4001, 2, 29), 5, 6, DOW_WED);
    assert(d.m_Year == 0 && d.m_Month == 0 && d.m_Day == 0);
    d = endDate(makeDate(2400000, 2, 29), 5, 6, DOW_WED);
    assert(d.m_Year == 0 && d.m_Month == 0 && d.m_Day == 0);

    //Day of week function
    int dow = GetDayOfWeek(makeDate(91230, 11, 17));
    assert(dow == 5);
    dow = GetDayOfWeek(makeDate(2024, 10, 1));
    assert(dow == 1);
    dow = GetDayOfWeek(makeDate(12378, 12, 17));
    assert(dow == 3);

    dow = GetDayOfWeek(makeDate(2023, 2, 28));
    assert(dow == 1);
    dow = GetDayOfWeek(makeDate(2023, 3, 1));
    assert(dow == 2);
    dow = GetDayOfWeek(makeDate(2023, 3, 2));
    assert(dow == 3);

    dow = GetDayOfWeek(makeDate(2024, 4, 1));
    assert(dow == 0);
    dow = GetDayOfWeek(makeDate(2024, 3, 31));
    assert(dow == 6);
    dow = GetDayOfWeek(makeDate(2024, 3, 10));
    assert(dow == 6);
    dow = GetDayOfWeek(makeDate(2024, 3, 7));
    assert(dow == 3);
    dow = GetDayOfWeek(makeDate(2024, 3, 2));
    assert(dow == 5);
    dow = GetDayOfWeek(makeDate(2024, 3, 1));
    assert(dow == 4);
    dow = GetDayOfWeek(makeDate(2024, 2, 28));
    assert(dow == 2);
    dow = GetDayOfWeek(makeDate(2024, 2, 29));
    assert(dow == 3);
    dow = GetDayOfWeek(makeDate(2000, 2, 29));
    assert(dow == 1);
    dow = GetDayOfWeek(makeDate(2000, 2, 28));
    assert(dow == 0);


    dow = GetDayOfWeek(makeDate(2002, 8, 1));
    assert(dow == 3);
    dow = GetDayOfWeek(makeDate(2001, 8, 1));
    assert(dow == 2);
    dow = GetDayOfWeek(makeDate(2000, 8, 1));
    assert(dow == 1);
    dow = GetDayOfWeek(makeDate(2000, 3, 1));
    assert(dow == 2);
    dow = GetDayOfWeek(makeDate(2000, 4, 1));
    assert(dow == 5);
    dow = GetDayOfWeek(makeDate(2000, 3, 14));
    assert(dow == 1);
    dow = GetDayOfWeek(makeDate(2000, 3, 7));
    assert(dow == 1);
    dow = GetDayOfWeek(makeDate(2000, 3, 2));
    assert(dow == 3);
    dow = GetDayOfWeek(makeDate(2000, 3, 3));
    assert(dow == 4);

    //Days between two days (border days included)
    assert(GetDayDifference(makeDate(2024, 10, 1), makeDate(2024, 10, 2)) == 2);
    assert(GetDayDifference(makeDate(2024, 10, 1), makeDate(2024, 11, 1)) == 32);
    assert(GetDayDifference(makeDate(2000, 10, 1), makeDate(3000, 10, 1)) == 365243);
    assert(GetDayDifference(makeDate(2000, 10, 1), makeDate(4000, 10, 1)) == 730485);
    assert(GetDayDifference(makeDate(2024, 10, 1), makeDate(10002024, 10, 1)) == 3652422501);


    //countConnections edge cases
    assert(countConnections ( makeDate ( 2024, 1, 2 ), makeDate ( 2024, 1, 5 ), 1, DOW_WEEKEND) == 0);
    assert(countConnections ( makeDate ( 2024, 1, 2 ), makeDate ( 2024, 1, 7 ), 1, DOW_MON) == 0);
    assert(countConnections ( makeDate ( 2024, 1, 2 ), makeDate ( 2024, 1, 7 ), 1, DOW_ALL) == 6);
    assert(countConnections ( makeDate ( 2023, 2, 29 ), makeDate ( 2024, 1, 7 ), 1, DOW_ALL) == -1);
    assert(countConnections ( makeDate ( 2023, 2, 28 ), makeDate ( 2025, 2, 29 ), 1, DOW_ALL) == -1);

    //date increment asserts
    d = IncrementDateByDays(makeDate(2024, 10, 1), 1);
    assert(d.m_Year == 2024 && d.m_Month == 10 && d.m_Day == 2);
    d = IncrementDateByDays(makeDate(2024, 10, 1), 31);
    assert(d.m_Year == 2024 && d.m_Month == 11 && d.m_Day == 1);
    d = IncrementDateByDays(makeDate(2025, 10, 1), 1);
    assert(d.m_Year == 2025 && d.m_Month == 10 && d.m_Day == 2);
    d = IncrementDateByDays(makeDate(2024, 10, 1), 365);
    assert(d.m_Year == 2025 && d.m_Month == 10 && d.m_Day == 1);
    d = IncrementDateByDays(makeDate(2024, 1, 1), 365);
    assert(d.m_Year == 2024 && d.m_Month == 12 && d.m_Day == 31);
    d = IncrementDateByDays(makeDate(2024, 1, 1), 366);
    assert(d.m_Year == 2025 && d.m_Month == 1 && d.m_Day == 1);
    d = IncrementDateByDays(makeDate(2000, 10, 1), 365243);
    assert(d.m_Year == 3000 && d.m_Month == 10 && d.m_Day == 2);
    d = IncrementDateByDays(makeDate(2000, 10, 1), 365243);
    assert(d.m_Year == 3000 && d.m_Month == 10 && d.m_Day == 2);

    d = IncrementDateByDays(makeDate(2000, 1, 1), 730484);
    assert(d.m_Year == 3999 && d.m_Month == 12 && d.m_Day == 31);

    d = IncrementDateByDays(makeDate(2000, 1, 1), 730485);
    assert(d.m_Year == 4000 && d.m_Month == 1 && d.m_Day == 1);

    d = IncrementDateByDays(makeDate(2000, 10, 1), 730484);
    assert(d.m_Year == 4000 && d.m_Month == 10 && d.m_Day == 1);

    d = IncrementDateByDays(makeDate(2000, 10, 1), 2556695);
    assert(d.m_Year == 9000 && d.m_Month == 10 && d.m_Day == 1);

    d = IncrementDateByDays(makeDate(2000, 1, 1), 7304850);
    assert(d.m_Year == 22000 && d.m_Month == 1 && d.m_Day == 6);


    const TDATE date = IncrementDateByDays(makeDate(2024, 10, 1), 3652422500);
    assert(date.m_Year == 10002024 && date.m_Month == 10 && date.m_Day == 1);
    d = IncrementDateByDays(makeDate(2024, 10, 1), 3652422257);
    assert(d.m_Year == 10002024 && d.m_Month == 2 && d.m_Day == 1);
    d = IncrementDateByDays(makeDate(2024, 10, 1), 3652422285);
    assert(d.m_Year == 10002024 && d.m_Month == 2 && d.m_Day == 29);
    d = IncrementDateByDays(makeDate(2024, 10, 1), 3652422286);
    assert(d.m_Year == 10002024 && d.m_Month == 3 && d.m_Day == 1);
    d = IncrementDateByDays(makeDate(2024, 10, 1), 3652421919);
    assert(d.m_Year == 10002023 && d.m_Month == 2 && d.m_Day == 28);
    d = IncrementDateByDays(makeDate(2024, 10, 1), 3652421920);
    assert(d.m_Year == 10002023 && d.m_Month == 3 && d.m_Day == 1);

    //default asserts
    assert(countConnections ( makeDate ( 2024, 10, 1 ), makeDate ( 2024, 10, 31 ), 1, DOW_ALL ) == 31);
    assert(countConnections ( makeDate ( 2024, 10, 1 ), makeDate ( 2024, 10, 31 ), 10, DOW_ALL ) == 266);
    assert(countConnections ( makeDate ( 2024, 10, 1 ), makeDate ( 2024, 10, 31 ), 1, DOW_WED ) == 5);
    assert(countConnections ( makeDate ( 2024, 10, 2 ), makeDate ( 2024, 10, 30 ), 1, DOW_WED ) == 5);
    assert(countConnections ( makeDate ( 2024, 10, 1 ), makeDate ( 2024, 10, 1 ), 10, DOW_TUE ) == 10);
    assert(countConnections ( makeDate ( 2024, 10, 1 ), makeDate ( 2024, 10, 1 ), 10, DOW_WED ) == 0);
    assert(
        countConnections ( makeDate ( 2024, 1, 1 ), makeDate ( 2034, 12, 31 ), 5, DOW_MON | DOW_FRI | DOW_SAT ) ==
        7462);
    assert(
        countConnections ( makeDate ( 2024, 1, 1 ), makeDate ( 2034, 12, 31 ), 0, DOW_MON | DOW_FRI | DOW_SAT ) == 0);
    assert(countConnections ( makeDate ( 2024, 1, 1 ), makeDate ( 2034, 12, 31 ), 100, 0 ) == 0);
    assert(countConnections ( makeDate ( 2024, 10, 10 ), makeDate ( 2024, 10, 9 ), 1, DOW_MON ) == -1);

    assert(countConnections ( makeDate ( 2024, 2, 29 ), makeDate ( 2024, 2, 29 ), 1, DOW_ALL ) == 1);

    assert(countConnections ( makeDate ( 2023, 2, 29 ), makeDate ( 2023, 2, 29 ), 1, DOW_ALL ) == -1);
    assert(countConnections ( makeDate ( 2100, 2, 29 ), makeDate ( 2100, 2, 29 ), 1, DOW_ALL ) == -1);
    assert(countConnections ( makeDate ( 2400, 2, 29 ), makeDate ( 2400, 2, 29 ), 1, DOW_ALL ) == 1);
    assert(countConnections ( makeDate ( 4000, 2, 29 ), makeDate ( 4000, 2, 29 ), 1, DOW_ALL ) == -1);

    d = endDate(makeDate(2024, 10, 1), 100, 1, DOW_ALL);
    assert(d . m_Year == 2025 && d . m_Month == 1 && d . m_Day == 8);
    d = endDate(makeDate(2024, 10, 1), 100, 6, DOW_ALL);
    assert(d . m_Year == 2024 && d . m_Month == 10 && d . m_Day == 20);
    d = endDate(makeDate(2024, 10, 1), 100, 1, DOW_WORKDAYS);
    assert(d . m_Year == 2025 && d . m_Month == 2 && d . m_Day == 17);
    d = endDate(makeDate(2024, 10, 1), 100, 4, DOW_WORKDAYS);
    assert(d . m_Year == 2024 && d . m_Month == 11 && d . m_Day == 4);
    d = endDate(makeDate(2024, 10, 1), 100, 1, DOW_THU);
    assert(d . m_Year == 2026 && d . m_Month == 9 && d . m_Day == 2);
    d = endDate(makeDate(2024, 10, 1), 100, 2, DOW_THU);
    assert(d . m_Year == 2025 && d . m_Month == 9 && d . m_Day == 17);
    d = endDate(makeDate(2024, 10, 1), 100, 0, DOW_THU);
    assert(d . m_Year == 0 && d . m_Month == 0 && d . m_Day == 0);
    d = endDate(makeDate(2024, 10, 1), 100, 1, 0);
    assert(d . m_Year == 0 && d . m_Month == 0 && d . m_Day == 0);

    d = endDate(makeDate(2024, 10, 2), 1, 10, DOW_TUE);
    assert(d . m_Year == 2024 && d . m_Month == 10 && d . m_Day == 7);

    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */


bool isValidDateSequence(TDATE first, TDATE second) {
    if (second.m_Year < first.m_Year) return false;

    if (second.m_Year == first.m_Year) {
        if (second.m_Month < first.m_Month) return false;

        if (second.m_Month == first.m_Month) {
            if (second.m_Day < first.m_Day) return false;
        }
    }

    return true;
}

bool isValidDate(TDATE date) {
    if (date.m_Month < 1 || date.m_Month > 12) return false;

    if (date.m_Day < 1) return false;

    unsigned char daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


    if (IsLeapYear(date.m_Year)) {
        daysInMonth[1] = 29;
        if (date.m_Day > daysInMonth[date.m_Month - 1]) return false;
    } else {
        if (date.m_Day > daysInMonth[date.m_Month - 1]) return false;
    }

    return true;
}

bool IsLeapYear(unsigned const year) {
    if (year % 4000 == 0) return false;
    if (year % 400 == 0) return true;
    if (year % 100 == 0) return false;
    if (year % 4 == 0) return true;
    return false;
}

int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}


unsigned char GetDayOfWeek(TDATE date) {
    unsigned short bonusDays = date.m_Year / 4000;

    if (date.m_Year > 2400) {
        date.m_Year = 2000 + date.m_Year % 400;
    }

    if (date.m_Month < 3) {
        date.m_Month += 12;
        date.m_Year--;
    }

    int dowForm = (date.m_Day + (13 * (date.m_Month + 1) / 5 + (date.m_Year % 100) + ((date.m_Year % 100) / 4) + (
                                     date.m_Year / 100 / 4) - 2 * (date.m_Year / 100)) + 5 - bonusDays);
    char dow = dowForm >= 0 ? dowForm % 7 : mod(dowForm, 7);

    return dow;
}

unsigned int GetConnectionsPerWeek(unsigned perWorkDay, unsigned dowMask, unsigned char startDow) {
    unsigned connectionsPerWeek = 0;

    for (; startDow < 5; startDow++) {
        if (dowMask & (1 << startDow)) {
            connectionsPerWeek += perWorkDay;
        }
    }

    if (startDow <= 5 && dowMask & 0b0100000) {
        connectionsPerWeek += ceil(perWorkDay / 2.0);
    }

    if (dowMask & 0b1000000) {
        connectionsPerWeek += ceil(perWorkDay / 3.0);
    }

    return connectionsPerWeek;
}

unsigned int GetConnectionsForDays(long long days, unsigned perWorkDay, unsigned dowMask, unsigned char startDow) {
    unsigned connections = 0;

    for (; startDow < 5; startDow++) {
        if (days == 0) {
            return connections;
        }
        if (dowMask & (1 << startDow)) {
            connections += perWorkDay;
        }
        days--;
    }

    if (startDow <= 5 && dowMask & 0b0100000 && days > 0) {
        connections += ceil(perWorkDay / 2.0);
        days--;
    }

    if (dowMask & 0b1000000 && days > 0) {
        connections += ceil(perWorkDay / 3.0);
    }

    return connections;
}


unsigned int GetConnectionsPerWeekInverted(unsigned perWorkDay, unsigned dowMask, unsigned char endDow) {
    unsigned connectionsPerWeek = 0;

    unsigned char n = endDow > 4 ? 4 : endDow;

    for (int i = 0; i <= n; i++) {
        if (dowMask & (1 << i)) {
            connectionsPerWeek += perWorkDay;
        }
    }

    if (endDow < 5) {
        return connectionsPerWeek;
    }

    if (dowMask & 0b0100000) {
        connectionsPerWeek += ceil(perWorkDay / 2.0);
    }

    if (endDow == 6 && dowMask & 0b1000000) {
        connectionsPerWeek += ceil(perWorkDay / 3.0);
    }

    return connectionsPerWeek;
}

long long GetDayDifference(TDATE from, TDATE to) {
    return GetTotalDaysInDate(to) - GetTotalDaysInDate(from) + 1;
}

long long GetTotalDaysInDate(TDATE date) {
    long long daysBefore = date.m_Year * 365 + date.m_Day;

    unsigned char daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    for (unsigned int i = 0; i < date.m_Month - 1; i++) {
        daysBefore += daysInMonth[i];
    }

    if (date.m_Month < 3)
        date.m_Year--;

    daysBefore += date.m_Year / 4 - date.m_Year / 100 + date.m_Year / 400 - date.m_Year / 4000;

    return daysBefore;
}


// long long GetTotalDaysInDate(TDATE date) {
//    const unsigned m = (date.m_Month + 9) % 12;
//     const unsigned y = date.m_Year - m / 10;
//     return 365 * y + y / 4 - y / 100 + y / 400 - y/4000 + (m * 306 + 5) / 10 + (date.m_Day - 1);
// }

int GetOffsetDays(int day, int month, int year) {
    int offset = day;

    unsigned char daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    for (int i = 0; i < month - 1; i++) {
        offset += daysInMonth[i];
    }

    // Adds one more day if leap year and past February
    if (IsLeapYear(year) && month > 2) {
        offset += 1;
    }

    return offset;
}

TDATE ConvertOffsetToDate(int offset, int year) {
    int daysInMonth[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (IsLeapYear(year)) {
        daysInMonth[2] = 29;
    }

    int month = 1;
    while (offset > daysInMonth[month] && month <= 12) {
        offset -= daysInMonth[month];
        month++;
    }

    int day = offset;
    return makeDate(year, month, day);
}

TDATE IncrementDateByDays(TDATE date, long long daysToAdd) {
    long long initialOffset = GetOffsetDays(date.m_Day, date.m_Month, date.m_Year);
    int remainingDaysInYear = IsLeapYear(date.m_Year) ? (366 - initialOffset) : (365 - initialOffset);

    int newYear;
    int newOffset;

    if (daysToAdd <= remainingDaysInYear) {
        newYear = date.m_Year;
        newOffset = initialOffset + daysToAdd;
    } else {
        daysToAdd -= remainingDaysInYear;
        newYear = date.m_Year + 1;

        while (true) {
            //Classy
            int daysInCurrentYear = IsLeapYear(newYear) ? 366 : 365;
            if (daysToAdd < daysInCurrentYear) {
                break;
            }
            daysToAdd -= daysInCurrentYear;
            newYear++;
        }

        newOffset = daysToAdd;
    }

    TDATE resultDate = ConvertOffsetToDate(newOffset, newYear);

    if (resultDate.m_Day == 0) {
        if (resultDate.m_Month == 1) {
            return makeDate(resultDate.m_Year - 1, 12, 31);
        }

        return makeDate(resultDate.m_Year, resultDate.m_Month - 1, 31);
    }

    return resultDate;
}


char GetLastPossibleDayInWeek(long long connections, unsigned perWorkDay, unsigned dowMask) {
    char endDow = -1;

    for (char i = 0; i < 5; i++) {
        if (!(dowMask & (1 << i))) {
            endDow = i;
            continue;
        }

        if (connections < perWorkDay) {
            return endDow;
        }

        connections -= perWorkDay;
        endDow = i;
    }

    if (dowMask & 0b0100000) {
        const unsigned int pwdSat = (unsigned int) ceil(perWorkDay / 2.0);

        if (connections < pwdSat) {
            return endDow;
        }

        connections -= pwdSat;
    }
    endDow = 5;

    if (dowMask & 0b1000000) {
        const unsigned int pwdSun = (unsigned int) ceil(perWorkDay / 3.0);

        if (connections < pwdSun) {
            return endDow;
        }

        //this should never happen? because otherwise we'd be in next week
        // connections -= pwdSun;
        // endDow = 6;
    }
    return 6;
}

long long GetDaysWhenConnectionsLessThanWeek(long long connections, unsigned perWorkDay, unsigned dowMask,
                                             unsigned char startDow) {
        long long days = 0;

    for (unsigned char i = startDow; i < 5; i++) {
        if (!(dowMask & (1 << i))) {
            days++;
            continue;
        }

        if (connections < perWorkDay) {
            return days;
        }

        connections -= perWorkDay;
        days++;
    }

    if (dowMask & 0b0100000) {
        const unsigned int pwdSat = (unsigned int) ceil(perWorkDay / 2.0);
        if (connections < pwdSat) {
            return days;
        }

        connections -= pwdSat;
    }
    days++;

    if (dowMask & 0b1000000) {
        const unsigned int pwdSun = (unsigned int) ceil(perWorkDay / 3.0);

        if (connections < pwdSun) {
            return days;
        }

        connections -= pwdSun;
    }
    days++;

    if (connections > 0) {
        days += GetDaysWhenConnectionsLessThanWeek(connections, perWorkDay, dowMask, 0);
    }
    return days;
}
