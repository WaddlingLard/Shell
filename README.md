# Project: Shell

* Author: Brian Wu
* Class: CS452 Section #002
* Semester: Spring 2025

## Overview

A shell built up from a skeleton with features including sequences, background tasks, redirections, pipelines, and several builtin commands.

## Reflection

The way to do this assignment was rather fun! It was challenging to thing about but the solutions to build up each feature were surprisingly intuitive given you understand how the program works!.

## Compiling and Using

To compile the code, first cd into the `hw2` directory.
Then, run this command 

```make```

and then...

```./shell```

OR... if you want to run immediately after building use

```make run```

Just make sure the file structure is not disturbed and all should be good!

Note: There is a regression tester that can be ran with ```make trytest```, but do note that some tests will not work because there expected output was ran on a machine that does not contain the same file path another machine would have when running the tester. Tread with caution!

NOTE: The tester does not like the shared object libdeq.so, but I have run it on my system (I know how cliche) and it works! If you don't trust me you can just do each test manually to verify. I'm sorry, the tester is rather finicky.

## Results 

The tests are promising and the shell works as intended. Certain tests from the regression tester won't work because they are certain to specific file paths. However, the functionality of the shell is complete and testing by hand would show that.

## Sources used

None
----------

## Notes

* This README template is using Markdown. Here is some help on using Markdown: 
[markdown cheatsheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet)


* Markdown can be edited and viewed natively in most IDEs such as Eclipse and VS Code. Just toggle
between the Markdown source and preview tabs.

* To preview your README.md output online, you can copy your file contents to a Markdown editor/previewer
such as [https://stackedit.io/editor](https://stackedit.io/editor).
