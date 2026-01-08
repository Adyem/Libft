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

#Class Mutex Requirements

Each class must own a recursive mutex that can be locked multiple times and must be unlocked the same number of times.

During error handling, do not lock the class mutex. Use the Errno module mutex wrapper instead via
`std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());` so error reporting does not re-lock the class mutex.

Every class must expose a helper function that provides direct access to its recursive mutex.
Document and implement this helper as a dedicated low-level interface intended for cases like
validating constructor error handling immediately after construction by manually locking and
unlocking the mutex to verify proper use.

#Errno and Error Stack Rules

Class and non-class error handling must use the shared error-stack types from the Errno module.
Classes must push errors to both the thread-local error stack and their class-only error stack.

When publishing or reading class error codes or error stacks, do not lock the class mutex; use
`std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());` instead.

Errors and successful completions are reported by pushing entries onto the appropriate error stack.
A function that pushes an entry must leave it on the stack.
The function that checks the entry is responsible for popping it.
Functions marked strictly for internal use must not push any errors on the stack.

If a function completes with an error entry still present on the stack,
it must not push a success entry.

If an error entry was reported and then popped because it was handled,
the function must push a success entry to report that it completed successfully.

If a function completes successfully without having pushed any entry,
it must push exactly one success entry.

Functions that report error text or terminate the process (such as ft_strerror, ft_perror, ft_exit, or any helper that exits)
must not modify the error stack.

#Errno Global Error Stack

Define and use a global error stack in the Errno module so non-class functions can record errors
without overwriting prior state, preserving older errors in the stack. Use helper functions for
all global stack access; do not touch the stack directly. Helpers must provide push, pop last,
pop newest, pop all, and fetch by index (1-based). The newest error is always at the lowest index,
and helpers must also support retrieving the most recent error and error string values using the
same ordering rules.

Only .cpp files must be prefixed with the name of the module they belong to.
For .hpp files, prefix only those meant for internal use with the module name.
Generic headers may use the module's name, while class headers should use the class name as the filename.

Update README.md only when the change is important for the end user, such as improved functionality or unavoidable undefined behavior. Bug fixes alone do not require README updates.

Code that relies on platform-specific features must place only the platform-dependent portions into helper functions in the Compatebility module.

#Build and Test Timing

Building the library and running the full test suite typically takes about two and a half minutes, possibly a little longer.
