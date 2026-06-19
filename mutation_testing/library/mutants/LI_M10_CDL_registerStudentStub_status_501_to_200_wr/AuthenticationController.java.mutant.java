package com.library.library.controller;

import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

/**
 * Authentication endpoints are DISABLED for automated testing.
 *
 * Reason:
 * - JWT and AuthenticationManager are removed from the security configuration
 * - These endpoints are not required for Schemathesis / RESTler / EvoMaster
 * - Keeping stubs avoids application startup failure
 */

@RestController
@CrossOrigin(origins = "*")
public class AuthenticationController {

    /**
     * Stub endpoint to keep backward compatibility.
     * Always returns 501 (Not Implemented).
     */
    @PostMapping("/authenticate")
    public ResponseEntity<?> authenticateStub() {
        return ResponseEntity
                .status(501)
                .body("Authentication disabled for testing");
    }

    /**
     * Stub endpoint for student registration.
     */
    @PostMapping("/register_student")
    public ResponseEntity<?> registerStudentStub() {
        return ResponseEntity
                .status(200) // [MUTANT LI_M10: CDL] 501 → 200
                .body("Authentication disabled for testing");
    }

    /**
     * Stub endpoint for admin registration.
     */
    @PostMapping("/register_admin")
    public ResponseEntity<?> registerAdminStub() {
        return ResponseEntity
                .status(501)
                .body("Authentication disabled for testing");
    }
}
