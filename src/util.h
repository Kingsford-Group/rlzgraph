#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "RLZgraph.h"
#include "RLZfact.h"
#include "SuffixTree.h"

static string Input_ref = "";
static string Input_strings = "";

void printHelp();
void printVersion();

bool parse_argument(int argc, char* argv[]);
vector<string> readFASTA(string filename);

#endif