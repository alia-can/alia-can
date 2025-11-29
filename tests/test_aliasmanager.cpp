#include "aliasmanager.hpp"
#include "shelldetector.hpp"
#include <cassert>
#include <iostream>
static void testValidateAliasName(){assert(AliasManager::validateAliasName("ll"));assert(AliasManager::validateAliasName("git_log"));assert(!AliasManager::validateAliasName(""));assert(!AliasManager::validateAliasName("with space"));}
static void testValidateCommand(){assert(AliasManager::validateCommand("ls -la"));assert(!AliasManager::validateCommand(""));}
static void testFormatAlias(){AliasManager m(ShellDetector::Shell::BASH);Alias a{"ll","ls -la"};auto f=m.formatAlias(a);assert(f.find("alias ll")!=std::string::npos);}
static void testParseAliasLine(){auto a=AliasManager::parseAliasLine("alias ll='ls -la'");assert(a.name=="ll");assert(a.command=="ls -la");}
static void testIsAliasLine(){assert(AliasManager::isAliasLine("alias ll='ls'"));assert(!AliasManager::isAliasLine("export X=1"));}
void test_aliasmanager(){std::cout<<"Running AliasManager tests...\n";testValidateAliasName();testValidateCommand();testFormatAlias();testParseAliasLine();testIsAliasLine();std::cout<<"✓ AliasManager tests passed!\n";}
