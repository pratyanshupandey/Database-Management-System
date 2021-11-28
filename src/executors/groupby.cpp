#include "global.h"

/**
 * @brief 
 * SYNTAX: <new_table> <- GROUP BY <grouping_attribute> FROM <table_name> RETURN MAX|MIN|SUM|AVG(<attribute>)
 */

bool syntacticParseGROUPBY()
{
    logger.log("syntacticParseGROUPBY");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[3] != "BY" || tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "RETURN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = GROUPBY;
    parsedQuery.groupRelationName = tokenizedQuery[6];
    parsedQuery.groupResultRelationName = tokenizedQuery[0];
    parsedQuery.groupGroupingAttribute = tokenizedQuery[4];

    string opstring = tokenizedQuery[8];

    if (opstring.back() != ')' || opstring.size() < 6)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    } 
    else
    {
        parsedQuery.groupOperatedAttribute = "";
        for (uint i = 4; i < opstring.size() - 1; i++)
            parsedQuery.groupOperatedAttribute += opstring[i];
    }

    if (opstring.compare(0, 4, "MAX(") == 0)
        parsedQuery.groupOperation = "MAX";
    else if (opstring.compare(0, 4, "MIN(") == 0)
        parsedQuery.groupOperation = "MIN";
    else if (opstring.compare(0, 4, "SUM(") == 0)
        parsedQuery.groupOperation = "SUM";
    else if (opstring.compare(0, 4, "AVG(") == 0)
        parsedQuery.groupOperation = "AVG";
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseGROUPBY()
{
    logger.log("semanticParseGROUPBY");

    if (tableCatalogue.isTable(parsedQuery.groupResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupGroupingAttribute, parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Grouping Column doesn't exist in relation" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupOperatedAttribute, parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Operated Column doesn't exist in relation" << endl;
        return false;
    }

    return true;
}


void executeGROUPBY()
{
    logger.log("executeGROUPBY");
    
    return;
}