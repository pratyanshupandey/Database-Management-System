#include "global.h"
/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 * NEW_SYNTAX: <new_relation_name> <- JOIN USING NESTED <table1>, <table2> ON <column1> <bin_op> <column2> BUFFER <buffer_size>
 * NEW_SYNTAX: <new_relation_name> <- JOIN USING PARTHASH <table1>, <table2> ON <column1> <bin_op> <column2> BUFFER <buffer_size>
 */
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 13 || tokenizedQuery[3] != "USING" || !(tokenizedQuery[4] != "NESTED" || tokenizedQuery[4] != "PARTHASH") || tokenizedQuery[7] != "ON" || tokenizedQuery[11] != "BUFFER")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[5];
    parsedQuery.joinSecondRelationName = tokenizedQuery[6];
    parsedQuery.joinFirstColumnName = tokenizedQuery[8];
    parsedQuery.joinSecondColumnName = tokenizedQuery[10];
    parsedQuery.joinAlgorithm = tokenizedQuery[4];

    string bufferString = tokenizedQuery[12];
    for (uint i = 0; i < bufferString.size(); i++)
        if(!isdigit(bufferString[i]))
        {
        cout << "SYNTAX ERROR" << endl;
        return false;
        }

    parsedQuery.joinBuffer = stoi(bufferString);

    string binaryOperator = tokenizedQuery[9];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.joinBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }

    if (parsedQuery.joinBuffer < 3)
    {
        cout << "SEMANTIC ERROR: Buffer Size too small for the algorithm." << endl;
        return false;
    }
    return true;
}

void executeNestedJOIN()
{
    logger.log("executeNestedJOIN");
    cout << "executeNestedJOIN" << endl;
    return;
}

void executeParthashJOIN()
{
    logger.log("executeParthashJOIN");
    cout << "executeParthashJOIN" << endl;
    return;
}


void executeJOIN()
{
    logger.log("executeJOIN");
    if (parsedQuery.joinAlgorithm == "NESTED")
        executeNestedJOIN();
    else if (parsedQuery.joinAlgorithm == "PARTHASH")
        executeParthashJOIN();
    return;
}