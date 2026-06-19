package com.library.library.controller;

import com.library.library.entity.BookStudent;
import com.library.library.entity.Request;
import com.library.library.entity.User;
import com.library.library.exception.UnavailableForGivenDatesException;
import com.library.library.service.BookStudentService;
import com.library.library.service.RequestService;
import com.library.library.dao.UserDAO;
import jakarta.transaction.Transactional;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * MODIFIED FOR TESTGEN - JWT authentication removed
 * Instead of extracting user from JWT token, we accept userId in request or use default test user
 * 
 * UPDATED: Auto-creates users if they don't exist (for TestGen compatibility)
 */
@RestController
@CrossOrigin(origins = "*")
public class RequestController {

    private RequestService requestService;
    private BookStudentService bookStudentService;

    @Autowired
    private UserDAO userDAO;

    // Default test user ID (created by TestDataInitializer)
    private static final int DEFAULT_TEST_USER_ID = 1;

    public RequestController(RequestService requestService, BookStudentService bookStudentService) {
        this.requestService = requestService;
        this.bookStudentService = bookStudentService;
    }

    // Get all requests
    @GetMapping("/requests/allRequests")
    public List<Request> getAllRequests() {
        return requestService.findAllRequests();
    }

    // Get requests by student id
    @GetMapping("/requests/student/{student_id}")
    public List<Request> getAllRequestsByStudentId(@PathVariable int student_id) {
        return requestService.getRequestbyStudentID(student_id);
    }

    // Get requests by request id
    @GetMapping("/requests/{id}")
    public Request getRequestById(@PathVariable int id) {
        return requestService.getRequestbyID(id);
    }

    // Get requests by book id
    @GetMapping("/requests/book/{id}")
    public List<Request> getAllRequestsByBookCode(@PathVariable int id) {
        return requestService.getRequestsByBookCode(id);
    }

    // Save request
    // MODIFIED: Accept userId as query param instead of JWT
    // UPDATED: Auto-creates user if it doesn't exist
    @PostMapping("/requests/save")
    public Request saveRequest(@RequestBody Request request,
                                @RequestParam(required = false) Integer userId) {
        request.setSlno(0);
        
        // Use provided userId or default test user
        Integer effectiveUserId = (userId != null) ? userId : DEFAULT_TEST_USER_ID;
        
        // Auto-create user if doesn't exist (for TestGen compatibility)
        User user = getOrCreateUser(effectiveUserId);
        
        if (request.getStudent() != null) {
            request.getStudent().setUser(user);
            request.getStudent().addRequest(request);
        }
        return requestService.saveRequest(request);
    }

    // Delete requests by request id
    @DeleteMapping("/requests/delete/{id}")
    public Request deleteRequest(@PathVariable Integer id) {
        return null; // [MUTANT LI_M02: SDL] deletion skipped
    }

    // Get all bookStudents (book issue infos / loans)
    @GetMapping("/bookStudent/getAll")
    public List<BookStudent> getAllBooksOfStudent() {
        return this.bookStudentService.getAllBookStudents();
    }

    // Get bookStudent by book student ID
    @GetMapping("/bookStudent/{id}")
    public BookStudent getBookStudentById(@PathVariable Integer id) {
        return bookStudentService.getBookStudentById(id);
    }

    // Get bookStudent by student Id
    @GetMapping("/bookStudent/students/{student_id}")
    public List<BookStudent> getByStudentId(@PathVariable Integer student_id) {
        List<BookStudent> list = bookStudentService.getBookStudentByStudentId(student_id);
        System.out.println(list);
        return list;
    }

    // Get bookStudent by book id
    @GetMapping("/bookStudent/book/{id}")
    public List<BookStudent> getBookStudentByBookId(@PathVariable Integer id) {
        return bookStudentService.getBookStudentByBookId(id);
    }

    // Save book student
    // UPDATED: Auto-creates user if needed
    @PostMapping("/bookStudent/save")
    public BookStudent saveBookStudent(@RequestBody BookStudent bookStudent,
                                        @RequestParam(required = false) Integer userId) {
        bookStudent.setSlno(0);
        
        // Ensure student has a user
        if (bookStudent.getStudent() != null) {
            Integer effectiveUserId = (userId != null) ? userId : DEFAULT_TEST_USER_ID;
            User user = getOrCreateUser(effectiveUserId);
            bookStudent.getStudent().setUser(user);
        }
        
        return bookStudentService.addNewBookStudentPair(bookStudent);
    }

    // Delete book student by bookStudent id
    @DeleteMapping("/bookStudent/delete/{id}")
    public void deleteBookStudentById(@PathVariable Integer id) {
        this.bookStudentService.deleteById(id);
    }

    // Accept request and save it as book student (loan)
    // MODIFIED: No JWT needed, user info comes from request object
    // UPDATED: Auto-creates user if it doesn't exist
    @PostMapping("/bookStudent/accept")
    @Transactional
    public BookStudent accept(@RequestBody Request request,
                               @RequestParam(required = false) Integer userId) {
        if (bookStudentService.doesRequestOverlap(request)) {
            throw new UnavailableForGivenDatesException();
        } else {
            BookStudent bs = new BookStudent();
            bs.setEndDate(request.getEndDate());
            bs.setStartDate(request.getStartDate());
            bs.setBook(request.getBook());
            bs.setStudent(request.getStudent());

            // Use provided userId or default test user
            Integer effectiveUserId = (userId != null) ? userId : DEFAULT_TEST_USER_ID;
            
            // Auto-create user if doesn't exist (for TestGen compatibility)
            User user = getOrCreateUser(effectiveUserId);
            
            if (bs.getStudent() != null) {
                bs.getStudent().setUser(user);
            }

            BookStudent bs1 = saveBookStudent(bs, null);
            requestService.deleteRequestbyId(request.getSlno());
            return bs1;
        }
    }
    
    /**
     * Helper method to get or create a user by ID.
     * If the user doesn't exist, creates a new test user.
     * This enables TestGen to use any userId without pre-creating users.
     */
    private User getOrCreateUser(Integer userId) {
        System.out.println("[RequestController] getOrCreateUser called with userId=" + userId);
        
        // First try to find by ID
        User existingById = userDAO.findById(userId).orElse(null);
        if (existingById != null) {
            System.out.println("[RequestController] Found user by ID: " + existingById.getUsername());
            return existingById;
        }
        System.out.println("[RequestController] No user found with ID=" + userId);
        
        // Generate username for this userId
        String username = "testuser" + userId;
        
        // Check if username already exists (might have different ID)
        User existingByUsername = userDAO.findByUsername(username);
        if (existingByUsername != null) {
            System.out.println("[RequestController] Found user by username: " + username + " with ID=" + existingByUsername.getId());
            return existingByUsername;
        }
        System.out.println("[RequestController] No user found with username=" + username);
        
        // Create new user with unique username using timestamp to avoid collisions
        String uniqueUsername = "testgen_" + System.currentTimeMillis();
        System.out.println("[RequestController] Creating new user with username=" + uniqueUsername);
        
        User newUser = new User();
        newUser.setUsername(uniqueUsername);
        newUser.setPassword("testpass");
        return userDAO.save(newUser);
    }
}