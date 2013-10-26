## SubtitleComposer Coding Style ##

### Indentation

1 tab is used for indentation

Tab, not spaces!

### Declaring variables

Declare each variable on a separate line

Avoid short or meaningless names (e.g. "a", "rbarr", "nughdeget")

Single character variable names are only okay for counters and temporaries, where the purpose of the variable is obvious

Wait when declaring a variable until it is needed

```C++
	// Wrong
	int a, b;
	char *c, *d;
	
	// Correct
	int height;
	int width;
	char *nameOfThis;
	char *nameOfThat;
```

Variables and functions start with a lower-case letter. Each consecutive word in a variable’s name starts with an upper-case letter

Avoid abbreviations

```C++
	// Wrong
	short Cntr;
	char ITEM_DELIM = '\t';
	
	// Correct
	short counter;
	char itemDelimiter = '\t';
```

Classes always start with an upper-case letter. Public classes start with a ‘Q’ (QRgb) followed by an upper case letter. Public functions most often start with a ‘q’ (qRgb).
Acronyms are camel-cased (e.g. QXmlStreamReader, not QXMLStreamReader).

### Whitespace

Use blank lines to group statements together where suited
Always use only one blank line
Do not use space after a keyword
Always use one single space before a curly brace:

```C++
	// Wrong
	if (foo){
	}
	
	// Correct
	if(foo) {
	}
```

For pointers or references, always use a single space between the type and ‘*’ or ‘&’, but no space between the ‘*’ or ‘&’ and the variable name:

```C++
	char *x;
	const QString &myString;
	const char * const y = "hello";
```

Surround binary operators with spaces
No space after a cast
Avoid C-style casts when possible

```C++
	// Wrong
	char* blockOfMemory = (char* ) malloc(data.size());
	
	// Correct
	char *blockOfMemory = reinterpret_cast<char *>(malloc(data.size()));
```

Do not put multiple statements on one line
By extension, use a new line for the body of a control flow statement:

```C++
	// Wrong
	if(foo) bar();
	
	// Correct
	if(foo)
		bar();
```

### Braces

Use attached braces: The opening brace goes on the same line as the start of the statement. If the closing brace is followed by another keyword, it goes into the same line as well:

```C++
	// Wrong
	if(codec)
	{
	}
	else
	{
	}
	
	// Correct
	if(codec) {
	} else {
	}
```

Exception: Function implementations and class declarations always have the left brace on the start of a line:

```C++
	static void foo(int g)
	{
		qDebug("foo: %i", g);
	}
	
	class Moo
	{
	};
```

Use curly braces only when the body of a conditional statement contains more than one line:

```C++
	// Wrong
	if(address.isEmpty()) {
		return false;
	}
	
	for(int i = 0; i < 10; ++i) {
		qDebug("%i", i);
	}
	
	// Correct
	if(address.isEmpty())
		return false;
	
	for(int i = 0; i < 10; ++i)
		qDebug("%i", i);
```

Exception 1: Use braces also if the parent statement covers several lines / wraps:

```C++
	// Correct
	if(address.isEmpty() || !isValid()
		|| !codec) {
		return false;
	}
```

Exception 2: Brace symmetry: Use braces also in if-then-else blocks where either the if-code or the else-code covers several lines:

```C++
	// Wrong
	if(address.isEmpty())
		return false;
	else {
		qDebug("%s", qPrintable(address));
		++it;
	}
	
	// Correct
	if(address.isEmpty()) {
		return false;
	} else {
		qDebug("%s", qPrintable(address));
		++it;
	}
	
	// Wrong
	if(a)
		if(b)
			...
		else
			...
	// Correct
	if(a) {
		if(b)
			...
		else
			...
	}
```

Use curly braces when the body of a conditional statement is empty

```C++
	// Wrong
	while(a);
	
	// Correct
	while(a) {}
```

### Parentheses

Use parentheses to group expressions:

```C++
	// Wrong
	if(a && b || c)
	
	// Correct
	if((a && b) || c)
	
	// Wrong
	a + b & c
	
	// Correct
	(a + b) & c
```

### Switch statements

The case labels are in the same column as the switch
Every case must have a break (or return) statement at the end or a comment to indicate that there’s intentionally no break, unless another case follows immediately.

```C++
	switch(myEnum) {
	case Value1:
		doSomething();
		break;
	case Value2:
	case Value3:
		doSomethingElse();
		// fall through
	default:
		defaultHandling();
		break;
	}
```

Jump statements (break, continue, return, and goto)

Do not put ‘else’ after jump statements:

```C++
	// Wrong
	if(thisOrThat)
		return;
	else
		somethingElse();
	
	// Correct
	if(thisOrThat)
		return;
	somethingElse();
```

Exception: If the code is inherently symmetrical, use of ‘else’ is allowed to visualize that symmetry

### Line breaks

Keep lines shorter than 100 characters; wrap if necessary
Commas go at the end of wrapped lines; operators start at the beginning of the new lines. An operator at the end of the line is easy to miss if the editor is too narrow.

```C++
	// Wrong
	if(longExpression +
		otherLongExpression +
		otherOtherLongExpression) {
	}
	
	// Correct
	if(longExpression
		+ otherLongExpression
		+ otherOtherLongExpression) {
	}
```

### Inheritance and the `virtual` keyword

When reimplementing a virtual method, do not put the `virtual` keyword in the header file.
On Qt5, annotate them with the [Q_DECL_OVERRIDE](http://qt-project.org/doc/qt-5.0/qtcore/qtglobal.html#Q_DECL_OVERRIDE) macro after the function declaration, just before the ‘;’ (or the ‘{’ ).

### General exception

When strictly following a rule makes your code look bad, feel free to break it
