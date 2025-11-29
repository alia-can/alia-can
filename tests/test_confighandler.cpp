#include "configfilehandler.hpp"
#include "backupmanager.hpp"
#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>
namespace fs=std::filesystem;
static std::string getTempTestFile(){char* d=getenv("TMPDIR");if(!d)d=const_cast<char*>("/tmp");return std::string(d)+"/alia-can-test-config";}
static void cleanupTestFile(){std::string f=getTempTestFile();if(fs::exists(f))fs::remove(f);for(const auto&e:fs::directory_iterator(fs::path(f).parent_path())){auto n=e.path().filename().string();if(n.find("alia-can-test-config")!=std::string::npos){try{fs::remove(e.path());}catch(...){} }}}
static void testLoadEmptyFile(){cleanupTestFile();ConfigFileHandler h(getTempTestFile(),ShellDetector::Shell::BASH);assert(h.loadAliases().empty());}
static void testAddAlias(){cleanupTestFile();ConfigFileHandler h(getTempTestFile(),ShellDetector::Shell::BASH);Alias a{"ll","ls -la"};assert(h.addAlias(a));auto v=h.loadAliases();assert(v.size()==1);}
static void testRemoveAlias(){cleanupTestFile();ConfigFileHandler h(getTempTestFile(),ShellDetector::Shell::BASH);h.addAlias({"ll","ls -la"});h.addAlias({"gs","git status"});h.removeAlias("ll");auto v=h.loadAliases();assert(v.size()==1);}
static void testMultipleAliases(){cleanupTestFile();ConfigFileHandler h(getTempTestFile(),ShellDetector::Shell::ZSH);std::vector<Alias> v={{"ll","ls -la"},{"la","ls -A"},{"l","ls -CF"},{"gs","git status"}};for(auto&a:v)h.addAlias(a);auto r=h.loadAliases();assert(r.size()==v.size());}
static void testValidationOnAdd(){cleanupTestFile();ConfigFileHandler h(getTempTestFile(),ShellDetector::Shell::BASH);assert(!h.addAlias({"bad name","ls"}));assert(!h.addAlias({"ll",""}));}
static void testBackupCreation(){cleanupTestFile();std::string f=getTempTestFile();ConfigFileHandler h(f,ShellDetector::Shell::BASH);BackupManager b(f);h.addAlias({"ll","ls -la"});std::string p=b.createBackup();assert(!p.empty());assert(fs::exists(p));}
static void testRestoreBackup(){cleanupTestFile();std::string f=getTempTestFile();ConfigFileHandler h(f,ShellDetector::Shell::BASH);BackupManager b(f);h.addAlias({"ll","ls -la"});std::string p=b.createBackup();h.addAlias({"gs","git status"});assert(b.restoreFromBackup(p));auto v=h.loadAliases();assert(v.size()==1);}
void test_confighandler(){std::cout<<"Running ConfigFileHandler tests...\n";testLoadEmptyFile();testAddAlias();testRemoveAlias();testMultipleAliases();testValidationOnAdd();testBackupCreation();testRestoreBackup();cleanupTestFile();std::cout<<"✓ ConfigFileHandler tests passed!\n";}
