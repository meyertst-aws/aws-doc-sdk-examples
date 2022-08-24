// Copyright Amazon.com, Inc. or its affiliates.All Rights Reserved. 
// SPDX - License - Identifier: Apache - 2.0

// snippet-start:[general.cpp.starter.main]

#include <iostream>

int main()
{
    char* error= "this should give a warning";
    char* st = error;
    int dummy = *nullptr;
    return 0;
}
// snippet-end:[general.cpp.starter.main]
