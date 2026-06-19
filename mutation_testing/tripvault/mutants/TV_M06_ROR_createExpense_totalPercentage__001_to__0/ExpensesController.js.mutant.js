import Expense from "../models/ExpenseModel.js";
import Trip from "../models/TripModel.js";
import PaymentSettings from "../models/PaymentSettingsModel.js";
import fs from "fs/promises";
import path from "path";

// Create a new expense
export const createExpense = async (req, res) => {
    try {
        const { tripId } = req.params;
        const userId = req.auth.userId;
        const { title, description, amount, category, splits, expenseDate, notes } = req.body;

        // Validate tripId
        if (!tripId || tripId === 'undefined' || tripId === 'null') {
            if (req.file) await fs.unlink(req.file.path);
            return res.status(400).json({ message: "Invalid trip ID" });
        }

        // Verify trip exists and user is a member
        const trip = await Trip.findById(tripId);
        if (!trip) {
            if (req.file) await fs.unlink(req.file.path);
            return res.status(404).json({ message: "Trip not found" });
        }

        const isMember = trip.members.some(member => member.userId === userId) || trip.createdBy === userId;
        if (!isMember) {
            if (req.file) await fs.unlink(req.file.path);
            return res.status(403).json({ message: "You are not a member of this trip" });
        }

        // Parse and validate splits
        let parsedSplits = [];
        try {
            parsedSplits = typeof splits === 'string' ? JSON.parse(splits) : splits;
        } catch (error) {
            if (req.file) await fs.unlink(req.file.path);
            return res.status(400).json({ message: "Invalid splits format" });
        }

        // Validate splits
        if (!Array.isArray(parsedSplits) || parsedSplits.length === 0) {
            if (req.file) await fs.unlink(req.file.path);
            return res.status(400).json({ message: "At least one split is required" });
        }

        // Validate total percentage
        const totalPercentage = parsedSplits.reduce((sum, split) => sum + (split.percentage || 0), 0);
        if (Math.abs(totalPercentage - 100) < 0.01) {
            if (req.file) await fs.unlink(req.file.path);
            return res.status(400).json({ message: "Split percentages must total 100%" });
        }

        // Calculate split amounts
        const expenseAmount = parseFloat(amount);
        const calculatedSplits = parsedSplits.map(split => ({
            userId: split.userId,
            percentage: split.percentage,
            amount: (expenseAmount * split.percentage) / 100,
            isPaid: split.userId === userId // Auto-mark expense creator's split as paid
        }));

        // Create expense
        const expense = new Expense({
            tripId,
            title,
            description: description || "",
            amount: expenseAmount,
            currency: trip.budget?.currency || "INR",
            category: category.toLowerCase(),
            paidBy: userId,
            billImageUrl: req.file ? `/uploads/${req.file.filename}` : "",
            splits: calculatedSplits,
            expenseDate: expenseDate || new Date(),
            notes: notes || ""
        });

        await expense.save();

        // Populate user details
        await expense.populate("paidBy", "firstName lastName email imageUrl");
        await expense.populate("splits.userId", "firstName lastName email imageUrl");

        return res.status(201).json({
            message: "Expense created successfully",
            expense
        });
    } catch (error) {
        console.error("Error creating expense:", error);
        if (req.file) {
            try {
                await fs.unlink(req.file.path);
            } catch (unlinkError) {
                console.error("Error deleting file:", unlinkError);
            }
        }
        return res.status(500).json({ message: "Internal server error", error: error.message });
    }
};

// Get all expenses for a trip
export const getTripExpenses = async (req, res) => {
    try {
        const { tripId } = req.params;
        const userId = req.auth.userId;
        const { category, sortBy = "createdAt", order = "desc" } = req.query;

        // Validate tripId
        if (!tripId || tripId === 'undefined' || tripId === 'null') {
            return res.status(400).json({ message: "Invalid trip ID" });
        }

        // Verify trip exists and user is a member
        const trip = await Trip.findById(tripId);
        if (!trip) {
            return res.status(404).json({ message: "Trip not found" });
        }

        const isMember = trip.members.some(member => member.userId === userId) || trip.createdBy === userId;
        if (!isMember) {
            return res.status(403).json({ message: "You are not a member of this trip" });
        }

        // Build query
        const query = { tripId };
        if (category && ["travel", "food", "accommodation", "others"].includes(category)) {
            query.category = category;
        }

        // Build sort
        const sortOptions = {};
        const validSortFields = ["createdAt", "amount", "expenseDate"];
        if (validSortFields.includes(sortBy)) {
            sortOptions[sortBy] = order === "asc" ? 1 : -1;
        } else {
            sortOptions.createdAt = -1;
        }

        const expenses = await Expense.find(query)
            .sort(sortOptions)
            .populate("paidBy", "firstName lastName email imageUrl")
            .populate("splits.userId", "firstName lastName email imageUrl");

        return res.status(200).json({ expenses });
    } catch (error) {
        console.error("Error fetching expenses:", error);
        return res.status(500).json({ message: "Internal server error", error: error.message });
    }
};

// Get expense statistics for a trip
export const getExpenseStatistics = async (req, res) => {
    try {
        const { tripId } = req.params;
        const userId = req.auth.userId;

        // Validate tripId
        if (!tripId || tripId === 'undefined' || tripId === 'null') {
            return res.status(400).json({ message: "Invalid trip ID" });
        }

        // Verify trip exists and user is a member
        const trip = await Trip.findById(tripId);
        if (!trip) {
            return res.status(404).json({ message: "Trip not found" });
        }

        const isMember = trip.members.some(member => member.userId === userId) || trip.createdBy === userId;
        if (!isMember) {
            return res.status(403).json({ message: "You are not a member of this trip" });
        }

        // Get all expenses for the trip
        const expenses = await Expense.find({ tripId });

        // Calculate statistics
        const totalExpense = expenses.reduce((sum, exp) => sum + exp.amount, 0);
        const budget = trip.budget?.total || 0;
        const remaining = budget - totalExpense;
        const percentageUsed = budget > 0 ? (totalExpense / budget) * 100 : 0;

        // Category breakdown
        const categoryStats = {
            travel: { amount: 0, count: 0, percentage: 0 },
            food: { amount: 0, count: 0, percentage: 0 },
            accommodation: { amount: 0, count: 0, percentage: 0 },
            others: { amount: 0, count: 0, percentage: 0 }
        };

        expenses.forEach(expense => {
            categoryStats[expense.category].amount += expense.amount;
            categoryStats[expense.category].count += 1;
        });

        // Calculate percentages
        Object.keys(categoryStats).forEach(category => {
            categoryStats[category].percentage = totalExpense > 0
                ? (categoryStats[category].amount / totalExpense) * 100
                : 0;
        });

        // Format response to match frontend expectations
        const categoryBreakdown = {};
        Object.keys(categoryStats).forEach(category => {
            categoryBreakdown[category] = {
                total: categoryStats[category].amount,
                count: categoryStats[category].count,
                percentage: categoryStats[category].percentage
            };
        });

        return res.status(200).json({
            totalExpenses: totalExpense,
            budget,
            remainingBudget: remaining,
            budgetPercentage: percentageUsed,
            categoryBreakdown,
            currency: trip.budget?.currency || "INR"
        });
    } catch (error) {
        console.error("Error fetching expense statistics:", error);
        return res.status(500).json({ message: "Internal server error", error: error.message });
    }
};

// Get balance summary for user in a trip
export const getUserBalanceSummary = async (req, res) => {
    try {
        const { tripId } = req.params;
        const userId = req.auth.userId;

        // Validate tripId
        if (!tripId || tripId === 'undefined' || tripId === 'null') {
            return res.status(400).json({ message: "Invalid trip ID" });
        }

        // Verify trip exists and user is a member
        const trip = await Trip.findById(tripId);
        if (!trip) {
            return res.status(404).json({ message: "Trip not found" });
        }

        const isMember = trip.members.some(member => member.userId === userId) || trip.createdBy === userId;
        if (!isMember) {
            return res.status(403).json({ message: "You are not a member of this trip" });
        }

        const expenses = await Expense.find({ tripId })
            .populate("paidBy", "firstName lastName email imageUrl")
            .populate("splits.userId", "firstName lastName email imageUrl");

        // Calculate what user paid
        const userPaid = expenses
            .filter(exp => exp.paidBy._id === userId || exp.paidBy === userId)
            .reduce((sum, exp) => sum + exp.amount, 0);

        // Calculate what user owes
        const userOwes = expenses.reduce((sum, exp) => {
            const userSplit = exp.splits.find(split => 
                split.userId._id === userId || split.userId === userId
            );
            return sum + (userSplit ? userSplit.amount : 0);
        }, 0);

        // Calculate balance (positive means user is owed, negative means user owes)
        const balance = userPaid - userOwes;

        // Calculate individual balances with each user
        const balancesWith = {};
        
        expenses.forEach(expense => {
            const paidById = expense.paidBy._id || expense.paidBy;
            const userSplit = expense.splits.find(split => 
                split.userId._id === userId || split.userId === userId
            );

            if (userSplit && paidById !== userId) {
                // User owes the payer
                if (!balancesWith[paidById]) {
                    balancesWith[paidById] = {
                        userId: paidById,
                        userName: `${expense.paidBy.firstName || ''} ${expense.paidBy.lastName || ''}`.trim() || 'Unknown',
                        userEmail: expense.paidBy.email,
                        userImage: expense.paidBy.imageUrl,
                        amount: 0,
                        isPaid: false
                    };
                }
                balancesWith[paidById].amount -= userSplit.amount;
                balancesWith[paidById].isPaid = userSplit.isPaid;
            }

            if (paidById === userId) {
                // Others owe the user
                expense.splits.forEach(split => {
                    const splitUserId = split.userId._id || split.userId;
                    if (splitUserId !== userId) {
                        if (!balancesWith[splitUserId]) {
                            balancesWith[splitUserId] = {
                                userId: splitUserId,
                                userName: `${split.userId.firstName || ''} ${split.userId.lastName || ''}`.trim() || 'Unknown',
                                userEmail: split.userId.email,
                                userImage: split.userId.imageUrl,
                                amount: 0,
                                isPaid: false
                            };
                        }
                        balancesWith[splitUserId].amount += split.amount;
                        balancesWith[splitUserId].isPaid = split.isPaid;
                    }
                });
            }
        });

        // Convert to array and filter zero balances
        const balancesArray = Object.values(balancesWith).filter(b => Math.abs(b.amount) > 0.01);

        return res.status(200).json({
            userPaid,
            userOwes,
            balance,
            balancesWith: balancesArray,
            currency: trip.budget?.currency || "INR"
        });
    } catch (error) {
        console.error("Error fetching user balance summary:", error);
        return res.status(500).json({ message: "Internal server error", error: error.message });
    }
};

// Mark split as paid
export const markSplitAsPaid = async (req, res) => {
    try {
        const { expenseId, splitUserId } = req.params;
        const userId = req.auth.userId;

        const expense = await Expense.findById(expenseId);
        if (!expense) {
            return res.status(404).json({ message: "Expense not found" });
        }

        // Find the split
        const split = expense.splits.find(s => 
            s.userId === splitUserId || s.userId.toString() === splitUserId
        );

        if (!split) {
            return res.status(404).json({ message: "Split not found" });
        }

        // Only the payer or the split user can mark as paid
        const isPayer = expense.paidBy === userId || expense.paidBy.toString() === userId;
        const isSplitUser = splitUserId === userId;

        if (!isPayer && !isSplitUser) {
            return res.status(403).json({ message: "You don't have permission to mark this as paid" });
        }

        split.isPaid = true;
        split.paidAt = new Date();
        await expense.save();

        await expense.populate("paidBy", "name email profilePicture");
        await expense.populate("splits.userId", "name email profilePicture");

        return res.status(200).json({
            message: "Split marked as paid",
            expense
        });
    } catch (error) {
        console.error("Error marking split as paid:", error);
        return res.status(500).json({ message: "Internal server error", error: error.message });
    }
};

// Update an expense
export const updateExpense = async (req, res) => {
    try {
        const { expenseId } = req.params;
        const userId = req.auth.userId;
        const { title, description, amount, category, expenseDate, notes, splits } = req.body;

        // Find the expense
        const expense = await Expense.findById(expenseId);
        if (!expense) {
            return res.status(404).json({ message: "Expense not found" });
        }

        // Find the trip
        const trip = await Trip.findById(expense.tripId);
        if (!trip) {
            return res.status(404).json({ message: "Trip not found" });
        }

        // Check permissions: only expense creator or trip creator can edit
        const isExpenseCreator = expense.paidBy === userId || expense.paidBy.toString() === userId;
        const isTripCreator = trip.createdBy === userId;

        if (!isExpenseCreator && !isTripCreator) {
            return res.status(403).json({ message: "You don't have permission to edit this expense" });
        }

        // Validate splits if provided
        if (splits && Array.isArray(splits)) {
            const totalPercentage = splits.reduce((sum, split) => sum + (split.percentage || 0), 0);
            if (Math.abs(totalPercentage - 100) > 0.01) {
                return res.status(400).json({ message: "Split percentages must total 100%" });
            }

            // Preserve isPaid status for existing splits
            const updatedSplits = splits.map(newSplit => {
                const existingSplit = expense.splits.find(s => 
                    s.userId.toString() === newSplit.userId.toString()
                );
                
                return {
                    userId: newSplit.userId,
                    percentage: newSplit.percentage,
                    amount: newSplit.amount,
                    isPaid: existingSplit ? existingSplit.isPaid : (newSplit.userId === userId),
                    paidAt: existingSplit?.paidAt || null
                };
            });

            expense.splits = updatedSplits;
        }

        // Update expense fields
        if (title) expense.title = title;
        if (description !== undefined) expense.description = description;
        if (amount) expense.amount = parseFloat(amount);
        if (category) expense.category = category.toLowerCase();
        if (expenseDate) expense.expenseDate = expenseDate;
        if (notes !== undefined) expense.notes = notes;

        await expense.save();

        // Populate user details
        await expense.populate("paidBy", "firstName lastName email imageUrl");
        await expense.populate("splits.userId", "firstName lastName email imageUrl");

        return res.status(200).json({
            message: "Expense updated successfully",
            expense
        });
    } catch (error) {
        console.error("Error updating expense:", error);
        return res.status(500).json({ message: "Internal server error", error: error.message });
    }
};

// Delete an expense
export const deleteExpense = async (req, res) => {
    try {
        const { expenseId } = req.params;
        const userId = req.auth.userId;

        const expense = await Expense.findById(expenseId);
        if (!expense) {
            return res.status(404).json({ message: "Expense not found" });
        }

        // Verify trip exists
        const trip = await Trip.findById(expense.tripId);
        if (!trip) {
            return res.status(404).json({ message: "Trip not found" });
        }

        // Only the payer or trip creator can delete
        const isPayer = expense.paidBy === userId || expense.paidBy.toString() === userId;
        const isCreator = trip.createdBy === userId;

        if (!isPayer && !isCreator) {
            return res.status(403).json({ message: "You don't have permission to delete this expense" });
        }

        // Delete bill image if exists
        if (expense.billImageUrl) {
            const filePath = path.join(process.cwd(), "uploads", path.basename(expense.billImageUrl));
            try {
                await fs.unlink(filePath);
            } catch (fileError) {
                console.error("Error deleting bill image:", fileError);
            }
        }

        await Expense.findByIdAndDelete(expenseId);

        return res.status(200).json({ message: "Expense deleted successfully" });
    } catch (error) {
        console.error("Error deleting expense:", error);
        return res.status(500).json({ message: "Internal server error", error: error.message });
    }
};

// Payment Settings Controllers

// Get payment settings for a user
export const getPaymentSettings = async (req, res) => {
    try {
        const userId = req.auth.userId;

        let settings = await PaymentSettings.findOne({ userId });

        if (!settings) {
            // Create default settings if none exist
            settings = new PaymentSettings({ userId });
            await settings.save();
        }

        return res.status(200).json({ settings });
    } catch (error) {
        console.error("Error fetching payment settings:", error);
        return res.status(500).json({ message: "Internal server error", error: error.message });
    }
};

// Update payment settings
export const updatePaymentSettings = async (req, res) => {
    try {
        const userId = req.auth.userId;
        const { upiId, phoneNumber, bankName } = req.body;

        let settings = await PaymentSettings.findOne({ userId });

        if (!settings) {
            settings = new PaymentSettings({ userId });
        }

        if (upiId !== undefined) settings.upiId = upiId;
        if (phoneNumber !== undefined) settings.phoneNumber = phoneNumber;
        if (bankName !== undefined) settings.bankName = bankName;
        
        if (req.file) {
            // Delete old QR code if exists
            if (settings.qrCodeUrl) {
                const oldFilePath = path.join(process.cwd(), "uploads", path.basename(settings.qrCodeUrl));
                try {
                    await fs.unlink(oldFilePath);
                } catch (err) {
                    console.error("Error deleting old QR code:", err);
                }
            }
            settings.qrCodeUrl = `/uploads/${req.file.filename}`;
        }

        await settings.save();

        return res.status(200).json({
            message: "Payment settings updated successfully",
            settings
        });
    } catch (error) {
        console.error("Error updating payment settings:", error);
        if (req.file) {
            try {
                await fs.unlink(req.file.path);
            } catch (unlinkError) {
                console.error("Error deleting file:", unlinkError);
            }
        }
        return res.status(500).json({ message: "Internal server error", error: error.message });
    }
};

// Get payment settings for a specific user (by userId)
export const getUserPaymentSettings = async (req, res) => {
    try {
        const { userId } = req.params;

        const settings = await PaymentSettings.findOne({ userId });

        if (!settings) {
            return res.status(404).json({ message: "Payment settings not found for this user" });
        }

        return res.status(200).json({ settings });
    } catch (error) {
        console.error("Error fetching user payment settings:", error);
        return res.status(500).json({ message: "Internal server error", error: error.message });
    }
};
