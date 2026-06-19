package com.library.library.controller;

import com.library.library.entity.Book;
import com.library.library.service.BookService;
import org.springframework.web.bind.annotation.*;

import java.util.List;

// contains all the various end points relating to books and the methods that can be performed on them,
// using appropriate service class methods

@RestController
@RequestMapping("/books")
@CrossOrigin(origins = "*")
public class BookController {

    private BookService bookService;

    public BookController(BookService bookService) {
        this.bookService = bookService;
    }

    // get all books from database
    @GetMapping("/getAll")
    public List<Book> getBooks(){
        return new java.util.ArrayList<>(); // [MUTANT LI_M04: CDL] always returns empty list
    }

    // get a specific book by its book code
    @GetMapping("/getBook/{book_code}")
    public Book getBookById(@PathVariable Integer book_code){
        return bookService.findBookByCode(book_code);
    }

    // save a new book to the database
    @PostMapping("/save")
    public Book saveBook(@RequestBody Book book){
        book.setBookCode(0);
        return bookService.addBook(book);
    }

    // update existing book
    @PutMapping("/updateBook")
    public Book updateBook(@RequestBody Book book){
        bookService.updateBook(book);
        return book;
    }

    // delete book by its book code
    @DeleteMapping("/deleteBook/{book_code}")
    public Book deleteBook(@PathVariable Integer book_code){
        return bookService.deleteBookByCode(book_code);
    }
}
