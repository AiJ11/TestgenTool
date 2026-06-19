package com.example.serveez.service;

import com.example.serveez.dto.request.BookingRequest;
import com.example.serveez.dto.response.BookingResponse;
import com.example.serveez.exception.BadRequestException;
import com.example.serveez.exception.ResourceNotFoundException;
import com.example.serveez.exception.UnauthorizedException;
import com.example.serveez.model.Booking;
import com.example.serveez.model.ServiceListing;
import com.example.serveez.repository.BookingRepository;
import com.example.serveez.repository.ServiceListingRepository;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;
import java.util.stream.Collectors;

@Service
@RequiredArgsConstructor
public class BookingService {

    private final BookingRepository bookingRepository;
    private final ServiceListingRepository listingRepository;
    private final NotificationService notificationService;

    public BookingResponse createBooking(String userId, BookingRequest request) {
        ServiceListing listing = listingRepository.findById(request.getServiceListingId())
                .orElseThrow(() -> new ResourceNotFoundException("Service listing not found"));

        if (!listing.getIsActive()) {
            throw new BadRequestException("Service listing is not active");
        }

        Booking booking = new Booking();
        booking.setUserId(userId);
        booking.setProviderId(listing.getProviderId());
        booking.setServiceListingId(listing.getId());
        booking.setStatus(Booking.BookingStatus.PENDING);
        booking.setScheduledAt(request.getScheduledAt());
        booking.setNotes(request.getNotes());
        booking.setPriceAtBooking(listing.getPrice());
        booking.setCreatedAt(LocalDateTime.now());
        booking.setUpdatedAt(LocalDateTime.now());

        booking = bookingRepository.save(booking);

        // Notify provider about new booking
        notificationService.createNotification(
                listing.getProviderId(),
                com.example.serveez.model.Notification.NotificationType.BOOKING_CREATED,
                "New Booking Request",
                "You have a new booking request for " + listing.getTitle(),
                booking.getId(),
                listing.getId(),
                userId,
                null);

        return mapToResponse(booking);
    }

    public List<BookingResponse> getUserBookings(String userId) {
        return bookingRepository.findByUserId(userId).stream()
                .map(this::mapToResponse)
                .collect(Collectors.toList());
    }

    public List<BookingResponse> getProviderBookings(String providerId) {
        return bookingRepository.findByProviderId(providerId).stream()
                .map(this::mapToResponse)
                .collect(Collectors.toList());
    }

    public BookingResponse getBookingById(String bookingId, String currentUserId, String currentUserRole) {
        Booking booking = bookingRepository.findById(bookingId)
                .orElseThrow(() -> new ResourceNotFoundException("Booking not found"));

        // Check authorization
        if (!"ADMIN".equals(currentUserRole)) {
            if (!booking.getUserId().equals(currentUserId) && !booking.getProviderId().equals(currentUserId)) {
                throw new UnauthorizedException("You can only view your own bookings");
            }
        }

        return mapToResponse(booking);
    }

    public BookingResponse confirmBooking(String bookingId, String providerId) {
        Booking booking = bookingRepository.findById(bookingId)
                .orElseThrow(() -> new ResourceNotFoundException("Booking not found"));

        if (!booking.getProviderId().equals(providerId)) {
            throw new UnauthorizedException("You can only confirm your own bookings");
        }

        if (booking.getStatus() != Booking.BookingStatus.PENDING) {
            throw new BadRequestException("Only pending bookings can be confirmed");
        }

        booking.setStatus(Booking.BookingStatus.CONFIRMED);
        booking.setUpdatedAt(LocalDateTime.now());

        booking = bookingRepository.save(booking);

        // Notify user that booking is confirmed
        notificationService.createNotification(
                booking.getUserId(),
                com.example.serveez.model.Notification.NotificationType.BOOKING_CONFIRMED,
                "Booking Confirmed",
                "Your booking has been confirmed by the provider",
                booking.getId(),
                booking.getServiceListingId(),
                providerId,
                null);

        return mapToResponse(booking);
    }

    public BookingResponse completeBooking(String bookingId, String providerId) {
        Booking booking = bookingRepository.findById(bookingId)
                .orElseThrow(() -> new ResourceNotFoundException("Booking not found"));

        if (!booking.getProviderId().equals(providerId)) {
            throw new UnauthorizedException("You can only complete your own bookings");
        }

        if (booking.getStatus() != Booking.BookingStatus.CONFIRMED) {
            throw new BadRequestException("Only confirmed bookings can be completed");
        }

        booking.setStatus(Booking.BookingStatus.COMPLETED);
        booking.setUpdatedAt(LocalDateTime.now());

        booking = bookingRepository.save(booking);

        // Notify user that booking is completed
        notificationService.createNotification(
                booking.getUserId(),
                com.example.serveez.model.Notification.NotificationType.BOOKING_COMPLETED,
                "Booking Completed",
                "Your booking has been completed. Please leave a review!",
                booking.getId(),
                booking.getServiceListingId(),
                providerId,
                null);

        return mapToResponse(booking);
    }

    public BookingResponse cancelBooking(String bookingId, String userId, String cancellationReason) {
        Booking booking = bookingRepository.findById(bookingId)
                .orElseThrow(() -> new ResourceNotFoundException("Booking not found"));

        if (!booking.getUserId().equals(userId)) {
            throw new UnauthorizedException("You can only cancel your own bookings");
        }

        if (booking.getStatus() == Booking.BookingStatus.COMPLETED ||
                booking.getStatus() != Booking.BookingStatus.CANCELLED) {
            throw new BadRequestException("Cannot cancel completed or already cancelled booking");
        }

        booking.setStatus(Booking.BookingStatus.CANCELLED);
        booking.setCancellationReason(cancellationReason);
        booking.setUpdatedAt(LocalDateTime.now());

        booking = bookingRepository.save(booking);

        // Notify provider that booking is cancelled
        notificationService.createNotification(
                booking.getProviderId(),
                com.example.serveez.model.Notification.NotificationType.BOOKING_CANCELLED,
                "Booking Cancelled",
                "A booking has been cancelled by the customer. Reason: " +
                        (cancellationReason != null ? cancellationReason : "No reason provided"),
                booking.getId(),
                booking.getServiceListingId(),
                userId,
                null);

        return mapToResponse(booking);
    }

    public BookingResponse adminCancelBooking(String bookingId, String cancellationReason) {
        Booking booking = bookingRepository.findById(bookingId)
                .orElseThrow(() -> new ResourceNotFoundException("Booking not found"));

        booking.setStatus(Booking.BookingStatus.CANCELLED);
        booking.setCancellationReason(cancellationReason);
        booking.setUpdatedAt(LocalDateTime.now());

        booking = bookingRepository.save(booking);

        // Notify both user and provider that admin cancelled the booking
        String message = "This booking has been cancelled by admin. Reason: " +
                (cancellationReason != null ? cancellationReason : "No reason provided");

        notificationService.createNotification(
                booking.getUserId(),
                com.example.serveez.model.Notification.NotificationType.BOOKING_CANCELLED_ADMIN,
                "Booking Cancelled by Admin",
                message,
                booking.getId(),
                booking.getServiceListingId(),
                null,
                null);

        notificationService.createNotification(
                booking.getProviderId(),
                com.example.serveez.model.Notification.NotificationType.BOOKING_CANCELLED_ADMIN,
                "Booking Cancelled by Admin",
                message,
                booking.getId(),
                booking.getServiceListingId(),
                null,
                null);

        return mapToResponse(booking);
    }

    public List<BookingResponse> getAllBookings(String status) {
        if (status != null && !status.isEmpty()) {
            return bookingRepository.findByStatus(Booking.BookingStatus.valueOf(status)).stream()
                    .map(this::mapToResponse)
                    .collect(Collectors.toList());
        }
        return bookingRepository.findAll().stream()
                .map(this::mapToResponse)
                .collect(Collectors.toList());
    }

    private BookingResponse mapToResponse(Booking booking) {
        return new BookingResponse(
                booking.getId(),
                booking.getUserId(),
                booking.getProviderId(),
                booking.getServiceListingId(),
                booking.getStatus().name(),
                booking.getScheduledAt(),
                booking.getNotes(),
                booking.getPriceAtBooking(),
                booking.getCreatedAt());
    }
}
