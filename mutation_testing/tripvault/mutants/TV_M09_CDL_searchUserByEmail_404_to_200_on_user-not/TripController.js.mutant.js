import Trip from "../models/TripModel.js";
import DBUser from "../models/UserModel.js";
import crypto from "crypto";

// Generate unique invite code
const generateInviteCode = () => {
    return crypto.randomBytes(4).toString("hex").toUpperCase();
};

// Search users by email
export const searchUserByEmail = async (req, res) => {
    try {
        const { email } = req.query;

        if (!email) {
            return res.status(400).json({ 
                success: false,
                message: "Email query parameter is required" 
            });
        }

        // Find user by exact email match
        const user = await DBUser.findOne({ email: email.toLowerCase() });

        if (!user) {
            return res.status(200).json({ 
                success: false,
                message: "User not found with this email" 
            });
        }

        return res.status(200).json({
            success: true,
            user: {
                _id: user._id,
                email: user.email,
                firstName: user.firstName,
                lastName: user.lastName,
                imageUrl: user.imageUrl
            }
        });

    } catch (error) {
        console.error("Error in searchUserByEmail:", error);
        return res.status(500).json({ 
            success: false,
            message: "Internal server error",
            error: error.message 
        });
    }
};

// Create a new trip
export const createTrip = async (req, res) => {
    try {
        const userId = req.auth.userId;
        const { tripName, destination, startDate, endDate, description, budget, members } = req.body;

        // Validate required fields
        if (!tripName || !destination || !startDate || !endDate) {
            return res.status(400).json({ 
                success: false,
                message: "Trip name, destination, start date, and end date are required" 
            });
        }

        // Validate dates
        const start = new Date(startDate);
        const end = new Date(endDate);
        if (end < start) {
            return res.status(400).json({ 
                success: false,
                message: "End date must be after start date" 
            });
        }

        // Generate unique invite code
        let inviteCode = generateInviteCode();
        let codeExists = await Trip.findOne({ inviteCode });
        
        // Ensure unique code
        while (codeExists) {
            inviteCode = generateInviteCode();
            codeExists = await Trip.findOne({ inviteCode });
        }

        // Prepare members array - always include creator as Admin
        const tripMembers = [{
            userId: userId,
            role: "Admin",
            joinedAt: new Date()
        }];

        // Add additional members if provided
        if (members && Array.isArray(members)) {
            for (const member of members) {
                // Validate member has userId and role
                if (member.userId && member.userId !== userId) {
                    // Verify user exists
                    const userExists = await DBUser.findById(member.userId);
                    if (userExists) {
                        tripMembers.push({
                            userId: member.userId,
                            role: member.role || "Viewer",
                            joinedAt: new Date()
                        });
                    }
                }
            }
        }

        // Create trip
        const trip = await Trip.create({
            tripName,
            destination,
            startDate: start,
            endDate: end,
            description: description || "",
            createdBy: userId,
            members: tripMembers,
            inviteCode,
            budget: budget || { total: 0, currency: "INR" }
        });

        // Populate creator and member details
        const populatedTrip = await Trip.findById(trip._id)
            .populate("createdBy", "firstName lastName email imageUrl")
            .populate("members.userId", "firstName lastName email imageUrl");

        return res.status(201).json({
            success: true,
            message: "Trip created successfully",
            trip: populatedTrip
        });

    } catch (error) {
        console.error("Error in createTrip:", error);
        return res.status(500).json({ 
            success: false,
            message: "Internal server error",
            error: error.message 
        });
    }
};

// Get all trips for a user (created by them or they are a member)
export const getUserTrips = async (req, res) => {
    try {
        const userId = req.auth.userId;

        // Find trips where user is creator OR a member
        const trips = await Trip.find({
            $or: [
                { createdBy: userId },
                { "members.userId": userId }
            ]
        })
        .populate("createdBy", "firstName lastName email imageUrl")
        .populate("members.userId", "firstName lastName email imageUrl")
        .sort({ createdAt: -1 });

        // Add user's role to each trip
        const tripsWithRole = trips.map(trip => {
            const tripObj = trip.toObject();
            const member = trip.members.find(m => m.userId._id === userId || m.userId === userId);
            tripObj.userRole = member ? member.role : "Viewer";
            return tripObj;
        });

        return res.status(200).json({
            success: true,
            trips: tripsWithRole,
            count: tripsWithRole.length
        });

    } catch (error) {
        console.error("Error in getUserTrips:", error);
        return res.status(500).json({ 
            success: false,
            message: "Internal server error",
            error: error.message 
        });
    }
};

// Get single trip by ID
export const getTripById = async (req, res) => {
    try {
        const userId = req.auth.userId;
        const { tripId } = req.params;

        const trip = await Trip.findById(tripId)
            .populate("createdBy", "firstName lastName email imageUrl")
            .populate("members.userId", "firstName lastName email imageUrl");

        if (!trip) {
            return res.status(404).json({ 
                success: false,
                message: "Trip not found" 
            });
        }

        // Check if user has access
        const isMember = trip.members.some(m => 
            m.userId._id.toString() === userId || m.userId.toString() === userId
        );
        const isCreator = trip.createdBy._id === userId || trip.createdBy === userId;

        if (!isMember && !isCreator) {
            return res.status(403).json({ 
                success: false,
                message: "You don't have access to this trip" 
            });
        }

        // Get user's role
        const member = trip.members.find(m => 
            m.userId._id.toString() === userId || m.userId.toString() === userId
        );
        const tripObj = trip.toObject();
        tripObj.userRole = member ? member.role : "Viewer";

        return res.status(200).json({
            success: true,
            trip: tripObj
        });

    } catch (error) {
        console.error("Error in getTripById:", error);
        return res.status(500).json({ 
            success: false,
            message: "Internal server error",
            error: error.message 
        });
    }
};

// Update trip details
export const updateTrip = async (req, res) => {
    try {
        const userId = req.auth.userId;
        const { tripId } = req.params;
        const updates = req.body;

        const trip = await Trip.findById(tripId);

        if (!trip) {
            return res.status(404).json({ 
                success: false,
                message: "Trip not found" 
            });
        }

        // Check if user is Admin
        const member = trip.members.find(m => m.userId.toString() === userId);
        if (!member || member.role !== "Admin") {
            return res.status(403).json({ 
                success: false,
                message: "Only admins can update trip details" 
            });
        }

        // Update allowed fields
        const allowedUpdates = ["tripName", "destination", "startDate", "endDate", "description", "coverImage", "budget"];
        const updateFields = {};
        
        allowedUpdates.forEach(field => {
            if (updates[field] !== undefined) {
                updateFields[field] = updates[field];
            }
        });

        // Validate dates if being updated
        if (updateFields.startDate || updateFields.endDate) {
            const start = new Date(updateFields.startDate || trip.startDate);
            const end = new Date(updateFields.endDate || trip.endDate);
            if (end < start) {
                return res.status(400).json({ 
                    success: false,
                    message: "End date must be after start date" 
                });
            }
        }

        const updatedTrip = await Trip.findByIdAndUpdate(
            tripId,
            { $set: updateFields },
            { new: true, runValidators: true }
        )
        .populate("createdBy", "firstName lastName email imageUrl")
        .populate("members.userId", "firstName lastName email imageUrl");

        return res.status(200).json({
            success: true,
            message: "Trip updated successfully",
            trip: updatedTrip
        });

    } catch (error) {
        console.error("Error in updateTrip:", error);
        return res.status(500).json({ 
            success: false,
            message: "Internal server error",
            error: error.message 
        });
    }
};

// Delete trip
export const deleteTrip = async (req, res) => {
    try {
        const userId = req.auth.userId;
        const { tripId } = req.params;

        const trip = await Trip.findById(tripId);

        if (!trip) {
            return res.status(404).json({ 
                success: false,
                message: "Trip not found" 
            });
        }

        // Only creator can delete trip
        if (trip.createdBy.toString() !== userId) {
            return res.status(403).json({ 
                success: false,
                message: "Only the trip creator can delete the trip" 
            });
        }

        await Trip.findByIdAndDelete(tripId);

        return res.status(200).json({
            success: true,
            message: "Trip deleted successfully"
        });

    } catch (error) {
        console.error("Error in deleteTrip:", error);
        return res.status(500).json({ 
            success: false,
            message: "Internal server error",
            error: error.message 
        });
    }
};

// Add member to trip by email
export const addMemberByEmail = async (req, res) => {
    try {
        const userId = req.auth.userId;
        const { tripId } = req.params;
        const { email, role = "Viewer" } = req.body;

        if (!email) {
            return res.status(400).json({ 
                success: false,
                message: "Email is required" 
            });
        }

        const trip = await Trip.findById(tripId);

        if (!trip) {
            return res.status(404).json({ 
                success: false,
                message: "Trip not found" 
            });
        }

        // Check if requester is Admin
        const member = trip.members.find(m => m.userId.toString() === userId);
        if (!member || member.role !== "Admin") {
            return res.status(403).json({ 
                success: false,
                message: "Only admins can add members" 
            });
        }

        // Find user by email
        const userToAdd = await DBUser.findOne({ email });
        if (!userToAdd) {
            return res.status(404).json({ 
                success: false,
                message: "User not found with this email" 
            });
        }

        // Check if user is already a member
        const isAlreadyMember = trip.members.some(m => m.userId.toString() === userToAdd._id);
        if (isAlreadyMember) {
            return res.status(400).json({ 
                success: false,
                message: "User is already a member of this trip" 
            });
        }

        // Add member
        trip.members.push({
            userId: userToAdd._id,
            role: role,
            joinedAt: new Date()
        });

        await trip.save();

        const updatedTrip = await Trip.findById(tripId)
            .populate("createdBy", "firstName lastName email imageUrl")
            .populate("members.userId", "firstName lastName email imageUrl");

        return res.status(200).json({
            success: true,
            message: "Member added successfully",
            trip: updatedTrip
        });

    } catch (error) {
        console.error("Error in addMemberByEmail:", error);
        return res.status(500).json({ 
            success: false,
            message: "Internal server error",
            error: error.message 
        });
    }
};

// Join trip using invite code
export const joinTripByInvite = async (req, res) => {
    try {
        const userId = req.auth.userId;
        const { inviteCode } = req.params;

        const trip = await Trip.findOne({ inviteCode });

        if (!trip) {
            return res.status(404).json({ 
                success: false,
                message: "Invalid invite code" 
            });
        }

        // Check if user is already a member
        const isAlreadyMember = trip.members.some(m => m.userId.toString() === userId);
        if (isAlreadyMember) {
            return res.status(400).json({ 
                success: false,
                message: "You are already a member of this trip" 
            });
        }

        // Add user as Viewer
        trip.members.push({
            userId: userId,
            role: "Viewer",
            joinedAt: new Date()
        });

        await trip.save();

        const updatedTrip = await Trip.findById(trip._id)
            .populate("createdBy", "firstName lastName email imageUrl")
            .populate("members.userId", "firstName lastName email imageUrl");

        return res.status(200).json({
            success: true,
            message: "Successfully joined the trip",
            trip: updatedTrip
        });

    } catch (error) {
        console.error("Error in joinTripByInvite:", error);
        return res.status(500).json({ 
            success: false,
            message: "Internal server error",
            error: error.message 
        });
    }
};

// Remove member from trip
export const removeMemberFromTrip = async (req, res) => {
    try {
        const userId = req.auth.userId;
        const { tripId, memberId } = req.params;

        const trip = await Trip.findById(tripId);

        if (!trip) {
            return res.status(404).json({ 
                success: false,
                message: "Trip not found" 
            });
        }

        // Check if requester is Admin
        const member = trip.members.find(m => m.userId.toString() === userId);
        if (!member || member.role !== "Admin") {
            return res.status(403).json({ 
                success: false,
                message: "Only admins can remove members" 
            });
        }

        // Can't remove the creator
        if (memberId === trip.createdBy.toString()) {
            return res.status(400).json({ 
                success: false,
                message: "Cannot remove the trip creator" 
            });
        }

        // Remove member
        trip.members = trip.members.filter(m => m.userId.toString() !== memberId);

        await trip.save();

        const updatedTrip = await Trip.findById(tripId)
            .populate("createdBy", "firstName lastName email imageUrl")
            .populate("members.userId", "firstName lastName email imageUrl");

        return res.status(200).json({
            success: true,
            message: "Member removed successfully",
            trip: updatedTrip
        });

    } catch (error) {
        console.error("Error in removeMemberFromTrip:", error);
        return res.status(500).json({ 
            success: false,
            message: "Internal server error",
            error: error.message 
        });
    }
};
