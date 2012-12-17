msvcfilt
========

**msvcfilt** is a command line utility inspired by the GNU binutils program [c++filt](http://linux.die.net/man/1/c++filt). It reads text from standard input, searches for decorated Visual C++ symbol names, replaces them with their undecorated equivalent, and prints the text with replacements to standard output. For example, `FOO ?func1@a@@AAEXH@Z BAR` becomes `FOO private: void __thiscall a::func1(int) BAR`.

A similar tool, [undname.exe](http://msdn.microsoft.com/en-us/library/5x49w699.aspx), is already part of the Visual C++ build tools. I only became aware of **undname** after I had developed **msvcfilt**, otherwise I probably would have just used **undname**.

There seem to be two notable differences between **msvcfilt** and **undname**:

* **undname** takes symbols either from a file, similar to **msvcfilt**, or directly as command line arguments. I did not find a way to let it read directly from standard input. **msvcfilt** only reads from standard input. Personally, I find the pipe-through usage of **msvcfilt** more convenient.
* **undname** lets you configure how it will undecorate the symbols, while **msvcfilt** will always do full undecoration. See `undname /show_flags` for the list of available flags.

Usage
-----

I usually use **msvcfilt** like this:

    dumpbin /IMPORTS <foo.exe> | msvcfilt | less

**msvcfilt** supports a single command line argument, `--keep`. When this flag is set, the decorated symbol will not be replaced; instead the undecorated symbol will be inserted after it, e.g. `FOO ?func1@a@@AAEXH@Z "private: void __thiscall a::func1(int)" BAR`.

Build
-----

A 32 bit build of **msvcfilt** is available at http://dl.dropbox.com/u/267889/github/msvcfilt.exe.

The repository also contains Visual Studio 2010 solution and project files. These can either be opened with the Visual Studio 2010 IDE, or used for building on the command line, for example with `devenv msvcfilt.sln /Build`.

License
-------

**msvcfilt** is licensed under the MIT license. See the LICENSE file in the repository for its full text.
