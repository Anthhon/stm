## Usage

### Terminal Status

The `TerminalStatus` struct represents the status of the terminal, including window size and cursor coordinates. It has the following members:

```c
typedef struct {
        size_t x_win_size; // Horizontal window size
        size_t y_win_size; // Vertical window size
        size_t x_cursor_coord; // X cursor coordinates
        size_t y_cursor_coord; // Y cursor coordinates
} TerminalStatus;
TerminalStatus term_info = { };
```

You can use the `TerminalStatus` struct to retrieve information about the terminal's window size and cursor coordinates, enabling you to build interactive terminal applications that adapt to the current terminal environment.

Please note that the values of the `TerminalStatus` struct members should be updated using appropriate functions, such as `termui_get_windows_size()` and `termui_get_pos(),` before accessing them, in-case your going to use a function which is gonna use these values, the library itself it's gonna call call these functions to initialize the struct.

Feel free to utilize the `TerminalStatus` struct in your termui library to obtain information about the terminal's status and incorporate it into your graphical and interactive functionalities. If you have any questions or need further assistance regarding the `TerminalStatus` struct, feel free to reach out to the project maintainers or open an issue on the GitHub repository.

### Drawing Borders

The `termui_draw_border` function allows you to draw a border using a specified character. Here's an example:

```c
termui_draw_border('*');
```

This will draw a border using asterisks `*` around the terminal window.

### Creating Boxes

You can create boxes with customizable dimensions and border characters using the `termui_box_create` function. Here's an example:

```c
termui_box_create(10, 5, '#');
```

This will create a box with dimensions of 10 columns and 5 rows, using hashes `#` as the border character.

You can also create boxes at specific coordinates using the `termui_box_create_at` function. Here's an example:

```c
termui_box_create_at(10, 5, 3, 2, '+');
```

This will create a box with dimensions of 10 columns and 5 rows, starting at column 3 and row 2, using plus signs `+` as the border character.

### Displaying Text Boxes

The `termui_text_box` function allows you to display a text box with a message inside and a border. Here's an example:

```c
char message[] = "Hello, termui!";
termui_text_box(message, '*', 1);
```

This will display a text box with the message "Hello, termui!" inside, surrounded by asterisks `*` as the border.

You can also display text boxes at specific coordinates using the `termui_text_box_at` function. Here's an example:

```c
char message[] = "Hello, termui!";
termui_text_box_at(message, '*', 1, 3, 2);
```

This will display a text box with the message "Hello, termui!" inside, starting at column 3 and row 2, surrounded by asterisks `*` as the border.

### Moving the Cursor

The following functions allow you to move the cursor within the terminal:

The `termui_go_left`, `termui_go_right`, `termui_go_up`, `termui_go_down` function moves the cursor to their respective directions from its current position.

### Retrieving the Cursor Position

The termui_get_pos function allows you to get the current cursor position within the terminal:

```c
termui_get_pos();
```

The cursor position can be accessed through the `term_info.x_cursor_coord` and `term_info.y_cursor_coord` into the `TerminalStatus` struct.

It stores the current cursor position in the term_info structure. The cursor position includes the X and Y coordinates of the cursor within the terminal.
