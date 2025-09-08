#Coding Guidelines

Use return ; for void functions. Include a space before the semicolon.
Use return (value); for non-void returns. Include a space before the opening parenthesis.
Do not use for loops, ternary operators, or switch statements.
Indent code using 4 spaces per level.
After declaring a class, indent access specifiers (private, public, protected) by 4 spaces and indent member declarations within them by 8 spaces.
In class declarations, place private members above public members and separate the sections with an empty line.
Function and variable names must use snake_case.
Use full variable names instead of short ones or single letters, dont use s or str use string.

Use Allman style braces (opening brace on a new line).
In classes, member variable names must start with an underscore (_).
Within class member functions, access members and other methods using the this keyword.
Template classes may define member functions in the same file as the class declaration, but
other classes must split declarations into .hpp files and definitions into .cpp files.
Do not define member function bodies inside the class declaration; place all definitions outside the class.
Every class must declare and define a constructor and destructor, even if they simply contain return ;.
Classes must track errors with a mutable _error_code member, a private set_error to update ft_errno, and public get_error and get_error_str helpers.

Any .hpp or .cpp file inside a submodule must have its filename prefixed with that submodule's name.

Always update README.md to reflect any code changes.
