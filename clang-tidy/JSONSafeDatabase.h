//===--- JSONSafeDatabase.h - ----------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  The JSONSafeDatabase finds safe types and functions databases supplied as 
//  a file 'safe_functions.json'.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLING_JSONSAFEDATABASE_H
#define LLVM_CLANG_TOOLING_JSONSAFEDATABASE_H

#include "clang/Basic/LLVM.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/FileMatchTrie.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"
#include <memory>
#include <string>
#include <vector>
#include <set>

namespace clang {
namespace tidy {

/// \brief A JSON based safe functions and types database.
///
/// JSON safe database files must contain a list of JSON objects which
/// provide the lists of safe types and functions from standard libraries:
/// [
///   { "functions": ["<aFunction>", "<otherFunction>"],
///     "types": ["<aType>", "<otherType>"]
///   },
///   { "functions": ["<aFunction>", "<otherFunction>"],
///     "types": ["<aType>", "<otherType>"]
///   },
///   ...
/// ]
///
enum class JSONCommandLineSyntax { Windows, Gnu, AutoDetect };
class JSONSafeDatabase : public clang::tooling::CompilationDatabase {
public:

  /// \brief Loads a compilation database from a build directory.
  ///
  /// Looks at the specified 'BuildDirectory' and creates a compilation database
  /// that allows to query compile commands for source files in the
  /// corresponding source tree.
  ///
  /// Returns NULL and sets ErrorMessage if we were not able to build up a
  /// compilation database for the build directory.
  ///
  /// FIXME: Currently only supports JSON compilation databases, which
  /// are named 'compile_commands.json' in the given directory. Extend this
  /// for other build types (like ninja build files).
  static std::unique_ptr<JSONSafeDatabase>
  loadFromDirectory(StringRef BuildDirectory, std::string &ErrorMessage);

  /// \brief Tries to detect a compilation database location and load it.
  ///
  /// Looks for a compilation database in all parent paths of file 'SourceFile'
  /// by calling loadFromDirectory.
  static std::unique_ptr<JSONSafeDatabase>
  autoDetectFromSource(StringRef SourceFile, std::string &ErrorMessage);

  /// \brief Tries to detect a compilation database location and load it.
  ///
  /// Looks for a compilation database in directory 'SourceDir' and all
  /// its parent paths by calling loadFromDirectory.
  static std::unique_ptr<JSONSafeDatabase>
  autoDetectFromDirectory(StringRef SourceDir, std::string &ErrorMessage);


  /// \brief Loads a JSON safe database from the specified folder.
  ///
  /// Returns NULL and sets ErrorMessage if the database could not be
  /// loaded from the given file.
  static std::unique_ptr<JSONSafeDatabase>
  loadFromDirectory2(StringRef Directory, std::string &ErrorMessage);


  /// \brief Loads a JSON safe database from the specified file.
  ///
  /// Returns NULL and sets ErrorMessage if the database could not be
  /// loaded from the given file.
  static std::unique_ptr<JSONSafeDatabase>
  loadFromFile(StringRef FilePath, std::string &ErrorMessage,
               JSONCommandLineSyntax Syntax);

  /// \brief Loads a JSON safe database from a data buffer.
  ///
  /// Returns NULL and sets ErrorMessage if the database could not be loaded.
  static std::unique_ptr<JSONSafeDatabase>
  loadFromBuffer(StringRef DatabaseString, std::string &ErrorMessage,
                 JSONCommandLineSyntax Syntax);

  /// \brief Returns all compile commands in which the specified file was
  /// compiled.
  ///
  /// FIXME: Currently FilePath must be an absolute path inside the
  /// source directory which does not have symlinks resolved.
  std::vector<clang::tooling::CompileCommand>
  getCompileCommands(StringRef FilePath) const override;

  /// \brief Returns the list of all files available in the compilation database.
  ///
  /// These are the 'file' entries of the JSON objects.
  std::vector<std::string> getAllFiles() const override;

  /// \brief Returns all compile commands for all the files in the compilation
  /// database.
  std::vector<clang::tooling::CompileCommand> getAllCompileCommands() const override;

  /// \brief Returns the set of safe types names.
  std::set<std::string> getTypes() const;
  /// \brief Returns the set of safe functions names.
  std::set<std::string> getFunctions() const;


private:
  /// \brief Constructs a JSON safe database on a memory buffer.
  JSONSafeDatabase(std::unique_ptr<llvm::MemoryBuffer> Database,
                          JSONCommandLineSyntax Syntax)
      : Database(std::move(Database)), Syntax(Syntax),
        YAMLStream(this->Database->getBuffer(), SM) {}

  /// \brief Parses the database file and creates the index.
  ///
  /// Returns whether parsing succeeded. Sets ErrorMessage if parsing
  /// failed.
  bool parse(std::string &ErrorMessage);

  // Tuple (directory, filename, commandline, output) where 'commandline'
  // points to the corresponding scalar nodes in the YAML stream.
  // If the command line contains a single argument, it is a shell-escaped
  // command line.
  // Otherwise, each entry in the command line vector is a literal
  // argument to the compiler.
  // The output field may be a nullptr.
  typedef std::tuple<llvm::yaml::ScalarNode *,
                     llvm::yaml::ScalarNode *,
                     std::vector<llvm::yaml::ScalarNode *>,
                     llvm::yaml::ScalarNode *> CompileCommandRef;

  /// \brief Converts the given array of CompileCommandRefs to CompileCommands.
  void getCommands(ArrayRef<CompileCommandRef> CommandsRef,
                   std::vector<clang::tooling::CompileCommand> &Commands) const;

  /// \brief Converts the given array of yaml refs to strings.
  void getValues(ArrayRef<llvm::yaml::ScalarNode*> Refs,
                   std::set<std::string> &Values) const;

  // Maps file paths to the compile command lines for that file.
  llvm::StringMap<std::vector<CompileCommandRef>> IndexByFile;

  /// All the compile commands in the order that they were provided in the
  /// JSON stream.
  std::vector<CompileCommandRef> AllCommands;
  std::vector<llvm::yaml::ScalarNode*> AllFunctions;
  std::vector<llvm::yaml::ScalarNode*> AllTypes;

  clang::tooling::FileMatchTrie MatchTrie;

  std::unique_ptr<llvm::MemoryBuffer> Database;
  JSONCommandLineSyntax Syntax;
  llvm::SourceMgr SM;
  llvm::yaml::Stream YAMLStream;
};

} // end namespace tidy
} // end namespace clang

#endif
