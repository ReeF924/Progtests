#include <locale.h>
#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

constexpr int PHONE_DIGITS = 10;

typedef struct TNode {
    char *m_Name;
    struct TNode *m_Child[PHONE_DIGITS];
} TNODE;

typedef struct {
    TNODE *m_Root;
    int m_Size;
} TPHONEBOOK;

#endif /* __PROGTEST__ */


typedef struct SingleNode {
    struct SingleNode *next;
    TNODE *phonePointer;
} SRNode;

typedef struct {
    SRNode *head;
} RevLinkedList;

bool isPhoneInputValid(const char* input) {
    const char* number = input;
    while (*number != '\0') {
        if (*number < '0' || *number > '9') {
            return false;
        }
        number++;
    }
    return true;
}

bool hasMoreChildsThan(const TNODE *node, int moreThan) {
    int counter = 0;

    for (int i = 0; i < PHONE_DIGITS && counter <= moreThan; i++) {
        if (node->m_Child[i] != nullptr) {
            counter++;
        }
    }

    return counter > moreThan;
}

bool isRecordUsed(const TNODE *node, int childMin) {
    if (node == nullptr) {
        return false;
    }

    if (node->m_Name != nullptr) {
        return true;
    }

    return hasMoreChildsThan(node, childMin);
}

void delDoubleLinkedList(RevLinkedList *list) {
    SRNode *current = list->head;
    while (current != nullptr) {
        SRNode *next = current->next;
        free(current);
        current = next;
    }

    list->head = nullptr;
}

void trimDoubleLinkedList(RevLinkedList *list) {
    if (list->head == nullptr || list->head->next == nullptr) {
        return;
    }

    SRNode *current = list->head->next;
    while (current != nullptr) {
        SRNode *next = current->next;
        free(current);
        current = next;
    }
    list->head->next = nullptr;
}

void addToDoubleLinkedList(RevLinkedList *list, TNODE *numberPointer) {
    if (list->head == nullptr) {
        list->head = (SRNode *) malloc(sizeof(SRNode));
        list->head->phonePointer = numberPointer;
        list->head->next = nullptr;
        return;
    }

    SRNode *current = list->head;

    list->head = (SRNode *) malloc(sizeof(SRNode));
    list->head->phonePointer = numberPointer;
    list->head->next = current;
}

void initNode(TNODE* node) {
    if (node == nullptr) {
        exit(2);
    }
    node->m_Name = nullptr;
    for (int i = 0; i < PHONE_DIGITS; i++) {
        node->m_Child[i] = nullptr;
    }
}


void delNode(TNODE *node) {
    if (node == nullptr) {
        return;
    }

    for (int i = 0; i < PHONE_DIGITS; i++) {
        delNode(node->m_Child[i]);
    }

    if (node->m_Name != nullptr) {
        free(node->m_Name);
    }

    free(node);
}


bool addPhone(TPHONEBOOK *book, const char *phone, const char *name) {

    if (!isPhoneInputValid(phone)) {
        return false;
    }


    TNODE *dbNode = book->m_Root;
    const char *number = phone;

    if (dbNode == nullptr) {
        book->m_Root = (TNODE *) malloc(sizeof(TNODE));;
        initNode(book->m_Root);
        dbNode = book->m_Root;
    }

    while (*number != '\0') {
        if (*number < '0' || *number > '9') {
            return false;
        }

        if (dbNode->m_Child[*number - '0'] == nullptr) {
            dbNode->m_Child[*number - '0'] = (TNODE *) malloc(sizeof(TNODE));
            initNode(dbNode->m_Child[*number - '0']);
        }

        dbNode = dbNode->m_Child[*number - '0'];
        number++;
    }

    if (dbNode->m_Name != nullptr) {
        free(dbNode->m_Name);
        dbNode->m_Name = (char *) malloc(strlen(name) + 1);
        strcpy(dbNode->m_Name, name);
        return true;
    }

    dbNode->m_Name = (char *) malloc(strlen(name) + 1);
    strcpy(dbNode->m_Name, name);
    book->m_Size++;

    return true;
}

void delBook(TPHONEBOOK *book) {
    if (book->m_Root == nullptr) {
        return;
    }

    delNode(book->m_Root);
    book->m_Root = nullptr;
    book->m_Size = 0;
}



bool delPhone(TPHONEBOOK *book, const char *phone) {
    //loop through the list
    //if record along the way has no name and only one child add it into singleList
    //if it doesn't delete the entire singleList a function for th
    //find target
    //remove free pointers from the singleList (including the nodes in singlelist)

    if (!isPhoneInputValid(phone)) {
        return false;
    }

    RevLinkedList list = {nullptr};

    TNODE *dbNode = book->m_Root;
    const char *number = phone;


    if (dbNode == nullptr) {
        return false;
    }

    while (*number != '\0') {

        if (dbNode->m_Child[*number - '0'] == nullptr) {
            delDoubleLinkedList(&list);
            return false;
        }

        dbNode = dbNode->m_Child[*number - '0'];


        addToDoubleLinkedList(&list, dbNode);
        number++;
    }

    book->m_Size--;

    SRNode *current = list.head;

    //todo probably? not sure if I should return false or continue with the deletion???
    if (current->phonePointer->m_Name == nullptr) {
        return false;
    }

    if (hasMoreChildsThan(current->phonePointer, 0)) {
        free(current->phonePointer->m_Name);
        current->phonePointer->m_Name = nullptr;
        return true;
    }
    number--;

    if (current->next == nullptr) {
        book->m_Root->m_Child[*number - '0'] = nullptr;
        free(current->phonePointer->m_Name);
        current->phonePointer->m_Name = nullptr;
        delDoubleLinkedList(&list);
        return true;
    }

    do {
        SRNode *next = current->next;
        if (current->phonePointer->m_Name != nullptr) {
            free(current->phonePointer->m_Name);
        }
        free(current->phonePointer);
        free(current);
        current = next;
        list.head = current;

        current->phonePointer->m_Child[*number - '0'] = nullptr;

        number--;

        if (isRecordUsed(current->phonePointer, 0)) {
            break;
        }
    } while (current->next != nullptr);

    if (!isRecordUsed(current->phonePointer, 0)) {
        book->m_Root->m_Child[*number - '0'] = nullptr;
        free(current->phonePointer);
        if (book->m_Size == 0 || !isRecordUsed(book->m_Root, 0)) {
            free(book->m_Root);
            book->m_Root = nullptr;
        }
    }
        delDoubleLinkedList(&list);
    return true;
}

const char *findPhone(TPHONEBOOK *book, const char *phone) {

    if (!isPhoneInputValid(phone)) {
        return nullptr;
    }

    TNODE *dbNode = book->m_Root;
    const char *number = phone;

    const char *location = nullptr;

    if (dbNode == nullptr) {
        return nullptr;
    }

    while (*number != '\0') {
        if (dbNode->m_Child[*number - '0'] == nullptr) {
            return location;
        }


        dbNode = dbNode->m_Child[*number - '0'];
        if (dbNode->m_Name != nullptr) {
            location = dbNode->m_Name;
        }

        number++;
    }
    return location;
}


#ifndef __PROGTEST__
int main() {
    TPHONEBOOK b = {nullptr, 0};
    char tmpStr[100];
    const char *name;
    assert(addPhone ( &b, "420", "Czech Republic" ));
    assert(addPhone ( &b, "42022435", "Czech Republic CVUT" ));
    assert(addPhone ( &b, "421", "Slovak Republic" ));
    assert(addPhone ( &b, "44", "Great Britain" ));
    strncpy(tmpStr, "USA", sizeof (tmpStr) - 1);
    assert(addPhone ( &b, "1", tmpStr ));
    strncpy(tmpStr, "Guam", sizeof (tmpStr) - 1);
    assert(addPhone ( &b, "1671", tmpStr ));
    assert(addPhone ( &b, "44", "United Kingdom" ));

    assert(b . m_Size == 6);
    assert(! b . m_Root -> m_Name);
    assert(! b . m_Root -> m_Child[0]);
    assert(! strcmp ( b . m_Root -> m_Child[1] -> m_Name, "USA" ));
    assert(! b . m_Root -> m_Child[1] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[2]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Name);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[2]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Name);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[0]);
    assert(! strcmp ( b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Name, "Guam" ));
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[2]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[2]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[2]);
    assert(! b . m_Root -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Name);
    assert(! b . m_Root -> m_Child[4] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Name);
    assert(! strcmp ( b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Name, "Czech Republic" ));
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Name);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Name);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[2]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Name);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[0]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[1]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[2]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Name);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[0]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[1]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[2]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[3]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[4]);
    assert(
        ! strcmp ( b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] ->
            m_Child[3]
            -> m_Child[5] -> m_Name, "Czech Republic CVUT" ));
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[0]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[1]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[2]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[3]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[4]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[5]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[6]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[7]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[8]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[9]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[6]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[7]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[8]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[9]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[4]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[5]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[6]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[7]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[8]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[9]);
    assert(! strcmp ( b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Name, "Slovak Republic" ));
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[2]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[2]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[3]);
    assert(! strcmp ( b . m_Root -> m_Child[4] -> m_Child[4] -> m_Name, "United Kingdom" ));
    assert(! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[2]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[5]);
    assert(! b . m_Root -> m_Child[6]);
    assert(! b . m_Root -> m_Child[7]);
    assert(! b . m_Root -> m_Child[8]);
    assert(! b . m_Root -> m_Child[9]);


    name = findPhone(&b, "420800123456");
    assert(!strcmp ( name, "Czech Republic" ));
    name = findPhone(&b, "420224351111");
    assert(!strcmp ( name, "Czech Republic CVUT" ));
    name = findPhone(&b, "42022435");
    assert(!strcmp ( name, "Czech Republic CVUT" ));
    name = findPhone(&b, "4202243");
    assert(!strcmp ( name, "Czech Republic" ));
    name = findPhone(&b, "420224343258985224");
    assert(!strcmp ( name, "Czech Republic" ));
    name = findPhone(&b, "42");
    assert(!name);
    name = findPhone(&b, "422");
    assert(!name);
    name = findPhone(&b, "4422");
    assert(!strcmp ( name, "United Kingdom" ));
    name = findPhone(&b, "16713425245763");
    assert(!strcmp ( name, "Guam" ));
    name = findPhone(&b, "123456789123456789");


    assert(!strcmp ( name, "USA" ));
    assert(delPhone ( &b, "420" ));
    assert(delPhone ( &b, "421" ));
    assert(delPhone ( &b, "44" ));
    assert(delPhone ( &b, "1671" ));
    assert(!delPhone ( &b, "1672" ));
    assert(!delPhone ( &b, "1671" ));
    name = findPhone(&b, "16713425245763");
    assert(!strcmp ( name, "USA" ));
    name = findPhone(&b, "4422");
    assert(!name);
    name = findPhone(&b, "420800123456");
    assert(!name);
    name = findPhone(&b, "420224351111");
    assert(!strcmp ( name, "Czech Republic CVUT" ));
    assert(b . m_Size == 2);
    assert(! b . m_Root -> m_Name);
    assert(! b . m_Root -> m_Child[0]);
    assert(! strcmp ( b . m_Root -> m_Child[1] -> m_Name, "USA" ));
    assert(! b . m_Root -> m_Child[1] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[2]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[1] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[2]);
    assert(! b . m_Root -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Name);
    assert(! b . m_Root -> m_Child[4] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Name);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Name);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Name);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Name);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[0]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[2]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Name);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[0]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[1]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[2]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Name);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[0]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[1]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[2]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[3]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[4]);
    assert(! strcmp ( b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] ->
        m_Child[3] -> m_Child[5] -> m_Name, "Czech Republic CVUT" ));
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[0]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[1]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[2]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[3]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[4]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[5]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[6]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[7]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[8]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[5] -> m_Child[9]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[6]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[7]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[8]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3]
        -> m_Child[9]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[4]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[5]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[6]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[7]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[8]);
    assert(
        ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[2]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[3]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[4]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[5]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[6]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[7]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[8]);
    assert(! b . m_Root -> m_Child[4] -> m_Child[9]);
    assert(! b . m_Root -> m_Child[5]);
    assert(! b . m_Root -> m_Child[6]);
    assert(! b . m_Root -> m_Child[7]);
    assert(! b . m_Root -> m_Child[8]);
    assert(! b . m_Root -> m_Child[9]);
    assert(delPhone ( &b, "1" ));
    assert(delPhone ( &b, "42022435" ));
    assert(!addPhone ( &b, "12345XYZ", "test" ));
    assert(b . m_Size == 0);
    assert(! b . m_Root);

    delBook(&b);
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
