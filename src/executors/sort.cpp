#include"global.h"
/**
 * @brief File contains method to process SORT commands.
 * 
 * syntax:
 * R <- SORT relation_name BY column_name IN sorting_order
 * 
 * sorting_order = ASC | DESC 
 * SYNTAX: <new_table_name> <- SORT <table_name> BY <column_name> IN ASC | DESC
 * <new_table_name> <- SORT <table_name> BY <column_name> IN ASC | DESC BUFFER <buffer_size
 */
bool syntacticParseSORT(){
    logger.log("syntacticParseSORT");

    if((tokenizedQuery.size() != 8 && tokenizedQuery.size() != 10) || tokenizedQuery[4] != "BY" || tokenizedQuery[6] != "IN")
    {
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }
    if(tokenizedQuery.size() == 10 && tokenizedQuery[8] != "BUFFER")
    {
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }

    parsedQuery.queryType = SORT;
    parsedQuery.sortResultRelationName = tokenizedQuery[0];
    parsedQuery.sortRelationName = tokenizedQuery[3];
    parsedQuery.sortColumnName = tokenizedQuery[5];
    string sortingStrategy = tokenizedQuery[7];

    if(sortingStrategy == "ASC")
        parsedQuery.sortingStrategy = ASC;
    else if(sortingStrategy == "DESC")
        parsedQuery.sortingStrategy = DESC;
    else{
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }

    if(tokenizedQuery.size() == 10)
    {
        string bufferString = tokenizedQuery[9];
        for (uint i = 0; i < bufferString.size(); i++)
            if(!isdigit(bufferString[i]))
            {
                cout << "SYNTAX ERROR" << endl;
                return false;
            }

        parsedQuery.sortBuffer = stoi(bufferString);
    }

    return true;
}

bool semanticParseSORT(){
    logger.log("semanticParseSORT");

    if(tableCatalogue.isTable(parsedQuery.sortResultRelationName))
    {
        cout<<"SEMANTIC ERROR: Resultant relation already exists"<<endl;
        return false;
    }

    if(!tableCatalogue.isTable(parsedQuery.sortRelationName))
    {
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    if(!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnName, parsedQuery.sortRelationName))
    {
        cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
        return false;
    }

    if(parsedQuery.sortBuffer < 3)
    {
        cout<<"SEMANTIC ERROR: Buffer Size too small for the algorithm."<<endl;
        return false;
    }

    return true;
}

void executeSORT(){
    logger.log("executeSORT");
    cout << parsedQuery.sortBuffer << endl;
    cout << parsedQuery.sortRelationName << endl;
    return;
}