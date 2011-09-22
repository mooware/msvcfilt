#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DbgHelp.h>

using namespace std;

/// The regex pattern to recognize a decorated symbol. Only a guess.
static const char *DECORATED_SYMBOL_PATTERN = "\\?[a-zA-Z0-9_@?$]+";

//------------------------------------------------------------------------------

/**
 * This class interacts with the DbgHelp symbol handler functions.
 *
 * It is designed as a Meyers' Singleton, so that SymInitialize() will only
 * be called if it is actually necessary, and that SymCleanup() will be called
 * at the end of the program.
 */
class SymbolHandler
{
public:
  /// The maximum length of a symbol name in bytes
  static const size_t MAX_SYMBOL_NAME_LEN = MAX_SYM_NAME;

  /**
   * Undecorates a decorated symbol.
   *
   * @param symbol The symbol to undecorate
   * @param result Receives the result
   *
   * @return True if the symbol was undecorated, otherwise false
   */
  bool UndecorateSymbol(const string &symbol, string &result)
  {
    DWORD res = UnDecorateSymbolName(symbol.c_str(), undecorateBuffer.get(),
                                     MAX_SYMBOL_NAME_LEN, UNDNAME_COMPLETE);

    bool success = (res != 0);
    if (success)
    {
      result = undecorateBuffer.get();
    }

    return success;
  }

  /// Returns the singleton instance of the class
  static SymbolHandler &GetInstance()
  {
    static SymbolHandler instance;
    return instance;
  }

private:
  /// True if the instance was successfully initialized
  bool initialized;
  /// Windows handle for the current process
  HANDLE hProc;
  /// Internal buffer that receives the undecorated symbols
  unique_ptr<char[]> undecorateBuffer;

  // no copy ctor, copy assignment
  SymbolHandler(const SymbolHandler &);
  SymbolHandler &operator=(const SymbolHandler &);

  /// Default constructor
  SymbolHandler()
    : initialized(false), hProc(0)
  {
    hProc = GetCurrentProcess();
    if (SymInitialize(hProc, NULL, FALSE) == TRUE)
    {
      initialized = true;

      // allocate the buffer that receives the undecorated symbols
      undecorateBuffer.reset(new char[MAX_SYMBOL_NAME_LEN]);
    }
  }

  /// Destructor
  ~SymbolHandler()
  {
    if (initialized)
    {
      SymCleanup(hProc);
    }
  }
};

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // if false, the decorated name will be replaced by the undecorated name.
  // is set by a command line option.
  bool keepOldName = false;

  // process cmdline arguments
  if (argc > 1)
  {
    for (int idx = 1; idx < argc; ++idx)
    {
      if (strcmp("-help", argv[idx]) == 0 ||
          strcmp("--help", argv[idx]) == 0)
      {
        cout << "Usage: msvcfilt [OPTIONS]..." << endl
             << "Searches in STDIN for Microsoft Visual C++ decorated symbol names" << endl
             << "and replaces them with their undecorated equivalent." << endl
             << endl
             << "Options:" << endl
             << "\t-help, --help\tDisplay this help and exit." << endl
             << "\t-keep, --keep\tDoes not replace the original, decorated symbol name." << endl
             << "\t             \tInstead, the undecorated name will be inserted after it." << endl
             << endl;

        return 0;
      }
      else if (strcmp("-keep", argv[idx]) == 0 ||
               strcmp("--keep", argv[idx]) == 0)
      {
        keepOldName = true;
      }
    }
  }

  // instantiate the regex pattern to search for
  regex pattern(DECORATED_SYMBOL_PATTERN);

  while (cin.good())
  {
    // read a line
    string line;
    getline(cin, line);

    // for every match, store the position and length of the original text,
    // and the string with which it will be replaced
    typedef tuple<size_t, size_t, string> replacement;
    vector<replacement> replacement_list;

    // iterate through the matches, store them and prepare the undecorated name
    const sregex_token_iterator end;
    for (sregex_token_iterator it(line.begin(), line.end(), pattern); it != end; ++it)
    {
      string result;

      bool success = SymbolHandler::GetInstance().UndecorateSymbol(it->str(), result);
      if (success)
      {
        tuple_element<0, replacement>::type pos = it->first - line.begin();
        tuple_element<1, replacement>::type len = it->length();

        replacement_list.push_back(make_tuple(pos, len, result));
      }
    }

    // now process the replacements. the vector is traversed in reverse so that
    // the positions in the original string stay valid.
    for (auto it = replacement_list.rbegin(); it != replacement_list.rend(); ++it)
    {
      // 0 : position in original string
      // 1 : length of text in original string
      // 2 : replacement string
      if (keepOldName)
      {
        auto insertText = get<2>(*it);
        insertText.insert(0, " \"", 2);
        insertText += '"';

        line.insert(get<0>(*it) + get<1>(*it), insertText);
      }
      else
      {
        line.replace(get<0>(*it), get<1>(*it), get<2>(*it));
      }
    }

    cout << line << endl;
  }
}
