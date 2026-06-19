package com.example.serveez.service;

import com.example.serveez.dto.request.LoginRequest;
import com.example.serveez.dto.request.SignupRequest;
import com.example.serveez.dto.response.AuthResponse;
import com.example.serveez.dto.response.UserResponse;
import com.example.serveez.exception.BadRequestException;
import com.example.serveez.exception.UnauthorizedException;
import com.example.serveez.model.User;
import com.example.serveez.repository.UserRepository;
import com.example.serveez.security.JwtUtil;
import jakarta.servlet.http.Cookie;
import jakarta.servlet.http.HttpServletResponse;
import lombok.RequiredArgsConstructor;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;

@Service
@RequiredArgsConstructor
public class AuthService {

    private final UserRepository userRepository;
    private final PasswordEncoder passwordEncoder;
    private final JwtUtil jwtUtil;

    @Value("${admin.email}")
    private String adminEmail;

    @Value("${admin.password}")
    private String adminPassword;

    @Value("${session.cookie.name:lsf_session}")
    private String cookieName;

    @Value("${session.cookie.max-age:86400}") // 24 hours
    private int cookieMaxAge;

    public AuthResponse signup(SignupRequest request, HttpServletResponse response) {
        if (userRepository.existsByEmail(request.getEmail())) {
            throw new BadRequestException("Email already exists");
        }

        User user = new User();
        user.setEmail(request.getEmail());
        user.setPasswordHash(passwordEncoder.encode(request.getPassword()));
        user.setRole(User.UserRole.valueOf(request.getRole()));
        user.setIsActive(true);
        user.setCreatedAt(LocalDateTime.now());
        user.setUpdatedAt(LocalDateTime.now());

        // [MUTANT SV_M02: SDL] userRepository.save() deleted

        String token = jwtUtil.generateToken(user.getId(), user.getRole().name());
        setAuthCookie(response, token);

        UserResponse userResponse = new UserResponse(
                user.getId(),
                user.getEmail(),
                user.getRole().name(),
                user.getIsActive(),
                user.getCreatedAt());

        return new AuthResponse("User registered successfully", userResponse);
    }

    public AuthResponse login(LoginRequest request, HttpServletResponse response) {
        User user = userRepository.findByEmail(request.getEmail())
                .orElseThrow(() -> new UnauthorizedException("Invalid email or password"));

        if (!passwordEncoder.matches(request.getPassword(), user.getPasswordHash())) {
            throw new UnauthorizedException("Invalid email or password");
        }

        if (!user.getIsActive()) {
            throw new UnauthorizedException("Account is inactive");
        }

        String token = jwtUtil.generateToken(user.getId(), user.getRole().name());
        setAuthCookie(response, token);

        UserResponse userResponse = new UserResponse(
                user.getId(),
                user.getEmail(),
                user.getRole().name(),
                user.getIsActive(),
                user.getCreatedAt());

        return new AuthResponse("Login successful", userResponse);
    }

    public AuthResponse adminLogin(LoginRequest request, HttpServletResponse response) {
        if (!adminEmail.equals(request.getEmail()) || !adminPassword.equals(request.getPassword())) {
            throw new UnauthorizedException("Invalid admin credentials");
        }

        // Find or create admin user
        User admin = userRepository.findByEmail(adminEmail)
                .orElseGet(() -> {
                    User newAdmin = new User();
                    newAdmin.setEmail(adminEmail);
                    newAdmin.setPasswordHash(passwordEncoder.encode(adminPassword));
                    newAdmin.setRole(User.UserRole.ADMIN);
                    newAdmin.setIsActive(true);
                    newAdmin.setCreatedAt(LocalDateTime.now());
                    newAdmin.setUpdatedAt(LocalDateTime.now());
                    return userRepository.save(newAdmin);
                });

        String token = jwtUtil.generateToken(admin.getId(), admin.getRole().name());
        setAuthCookie(response, token);

        UserResponse userResponse = new UserResponse(
                admin.getId(),
                admin.getEmail(),
                admin.getRole().name(),
                admin.getIsActive(),
                admin.getCreatedAt());

        return new AuthResponse("Admin login successful", userResponse);
    }

    public void logout(HttpServletResponse response) {
        Cookie cookie = new Cookie(cookieName, null);
        cookie.setMaxAge(0);
        cookie.setHttpOnly(true);
        cookie.setSecure(true);
        cookie.setPath("/");
        response.addCookie(cookie);
    }

    private void setAuthCookie(HttpServletResponse response, String token) {
        Cookie cookie = new Cookie(cookieName, token);
        cookie.setHttpOnly(true);
        cookie.setSecure(true);
        cookie.setMaxAge(cookieMaxAge);
        cookie.setPath("/");
        response.addCookie(cookie);
    }
}
