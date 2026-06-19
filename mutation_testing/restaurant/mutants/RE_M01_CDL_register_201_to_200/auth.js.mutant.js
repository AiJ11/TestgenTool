const express = require("express");
const jwt = require("jsonwebtoken");
const User = require("../models/User");
const { validateUser, validateLogin } = require("../middleware/validation");

const router = express.Router();

/**
 * @swagger
 * /api/auth/register:
 *   post:
 *     summary: Register a new user
 *     tags: [Auth]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               fullName:
 *                 type: string
 *               email:
 *                 type: string
 *               password:
 *                 type: string
 *               mobile:
 *                 type: string
 *               role:
 *                 type: string
 *                 enum: [customer, restaurant_owner, delivery_agent]
 *     responses:
 *       201:
 *         description: User registered successfully
 */
router.post("/register", validateUser, async (req, res) => {
  try {
    const { fullName, email, password, mobile, role } = req.body;

    // Check if user already exists
    const existingUser = await User.findOne({
      $or: [{ email }, { mobile }],
    });

    if (existingUser) {
      return res.status(400).json({
        message: "User with this email or mobile already exists",
      });
    }

    // Create new user
    const user = new User({
      fullName,
      email,
      password,
      mobile,
      role,
    });

    await user.save();

    // Generate JWT token
    const token = jwt.sign(
      { userId: user._id },
      process.env.JWT_SECRET || "your-secret-key",
      { expiresIn: "7d" }
    );

    // Save token to user document
    user.token = token;
    await user.save();

    res.status(200).json({
      message: "User registered successfully",
      token,
      user: {
        id: user._id,
        fullName: user.fullName,
        email: user.email,
        role: user.role,
      },
    });
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

/**
 * @swagger
 * /api/auth/login:
 *   post:
 *     summary: Login user
 *     tags: [Auth]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               email:
 *                 type: string
 *               password:
 *                 type: string
 *     responses:
 *       200:
 *         description: Login successful
 */
router.post("/login", validateLogin, async (req, res) => {
  console.log("[LOGIN] TEST_API_MODE:", process.env.TEST_API_MODE);
  // ... rest of the code
  try {
    const { email, password } = req.body;

    // Find user by email
    const user = await User.findOne({ email });
    if (!user || !user.isActive) {
      return res.status(401).json({ message: "Invalid credentials" });
    }

    // TEST MODE: PLAINTEXT PASSWORD CHECK
    if (process.env.TEST_API_MODE === "true") {
      if (user.password !== password) {
        return res.status(401).json({ message: "Invalid credentials" });
      }
    }
    // NORMAL MODE: HASHED PASSWORD CHECK
    else {
      const isMatch = await user.comparePassword(password);
      if (!isMatch) {
        return res.status(401).json({ message: "Invalid credentials" });
      }
    }

    // Generate JWT token
    const token = jwt.sign(
      { userId: user._id },
      process.env.JWT_SECRET || "your-secret-key",
      { expiresIn: "7d" }
    );

    // Save token to user document (important for test API)
    // Save token to user document (important for test API)
    user.token = token;
    await user.save();
    console.log(
      "[LOGIN] Saved token for:",
      email,
      "Token exists:",
      !!user.token
    );

    res.json({
      message: "Login successful",
      token,
      user: {
        id: user._id,
        fullName: user.fullName,
        email: user.email,
        role: user.role,
      },
    });
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

module.exports = router;
