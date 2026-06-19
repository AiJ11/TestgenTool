import dotenv from "dotenv";
import nodemailer from 'nodemailer';
import DBOTP from '../models/OtpModel.js';
import DBDevice from '../models/DevicesModel.js';
import DBUserDeviceLinks from '../models/UserDeviceLinksModel.js';
import { clerkClient } from '../utils/ClerkClient.js'
import DBUser from "../models/UserModel.js";
import DBSessions from "../models/SessionModel.js";
import { io, userDeviceManager } from "../socket.js";


dotenv.config();

const generate_html = (otp) => `
    <div style="font-family: Arial, sans-serif; max-width: 600px; margin: auto; border: 1px solid #e0e0e0; border-radius: 10px; overflow: hidden;">
      <div style="background-color: #6C28D9; color: white; padding: 20px; text-align: center;">
        <h1 style="margin: 0;">GhostSocket</h1>
        <p style="margin: 5px 0 0;">Secure Verification</p>
      </div>
      <div style="padding: 30px;">
        <p style="font-size: 16px; color: #333;">Hello,</p>
        <p style="font-size: 16px; color: #333;">
          Your One-Time Password (OTP) is:
        </p>
        <div style="background-color: #f3e8ff; border-left: 5px solid #6a0dad; padding: 15px; margin: 20px 0; text-align: center;">
          <span style="font-size: 24px; font-weight: bold; color: #6a0dad;">${otp}</span>
        </div>
        <p style="font-size: 14px; color: #666;">
          This OTP is valid for the next <strong>5 minutes</strong>. Please do not share it with anyone.
        </p>
        <p style="font-size: 14px; color: #999; margin-top: 40px;">
          - GhostSocket Security<br>
        </p>
      </div>
      <div style="background-color: #f9f9f9; padding: 15px; text-align: center; font-size: 12px; color: #aaa;">
        If you didn’t request this, you can safely ignore this email.
      </div>
    </div>
  `;

const transporter = nodemailer.createTransport({
  service: 'gmail',
  auth: {
    user: process.env.EMAIL_USER,
    pass: process.env.EMAIL_PASS,
  },
  tls: {
    rejectUnauthorized: false,
  }
});

export const checkUser = async (req, res) => {
  try {
    const { email } = req.body;

    // Check if the email is exists
    const users = await clerkClient.users.getUserList({ emailAddress: [email] });
    if (users.length === 0 || !users.data || !users.data[0]) {
      return res.status(404).json({ message: "User not found" });
    }
    // Check if the user has Google OAuth linked
    const user = users.data[0];
    const userData = await clerkClient.users.getUser(user.id);
    const externalAccounts = userData.externalAccounts;

    const hasGoogleAuth = externalAccounts.some(
      (account) => account.provider === "oauth_google"
    );

    if (hasGoogleAuth) {
      // If the user has Google OAuth linked, send an OTP to the email
      const otp = await sendOtp(email, user.id);
      if (otp.success) {
        return res.status(200).json({ type: "otp", oauth: "google" });
      } else {
        return res.status(500).json({ message: "Error sending OTP" });
      }
    } else {
      // If the user does not have Google OAuth linked, return email_password type
      return res.status(200).json({ type: "email_password" });
    }
  } catch (err) {
    console.error('Error:', err);
    return res.status(500).json({ message: "Internal server error" });
  }
};

export async function resendOtp(req, res) {
  try {
    const { email } = req.body;
    // Check if the email is exists
    const users = await clerkClient.users.getUserList({ emailAddress: [email] });
    if (users.length === 0 || !users.data || !users.data[0]) {
      return res.status(404).json({ message: "User not found" });
    }
    // Send OTP
    const userId = users.data[0].id;
    const otp = await sendOtp(email, userId);
    if (otp.success) {
      return res.status(200).json({ message: "Resent OTP!" });
    } else {
      return res.status(500).json({ message: "Error sending OTP" });
    }
  } catch (err) {
    console.error('Error:', err);
    return res.status(500).json({ message: "Internal server error" });
  }
}

export async function sendOtp(email, userId) {
  // Generate 6-digit OTP
  const otp = Math.floor(100000 + Math.random() * 900000).toString();

  const mailOptions = {
  from: process.env.EMAIL_USER,
  to: email,
  subject: 'Your GhostSocket OTP Code',
  html: generate_html(otp),
  };


  try {
    await transporter.sendMail(mailOptions);
    await DBOTP.create({ email, otp, expiresAt: Date.now() + 5 * 60 * 1000 , userId});
    return { success: true};
  } catch (err) {
    console.error('Error sending OTP:', err);
    return { success: false };
  }
}

export const verifyOtp = async (req, res) => {
  try {
    const { email, otp, deviceId } = req.body;

    // Check if the otp is correct and not expired
    const otpData = await DBOTP.findOne({ email, otp });
    if (!otpData) {
      return res.status(400).json({ message: "Invalid OTP" });
    }
    if (otpData.expiresAt < Date.now()) {
      return res.status(400).json({ message: "OTP expired" });
    }
    await DBOTP.deleteOne({ email, otp });

    // Check if the device id is already in the database
    const device = await DBDevice.findOne({ _id: deviceId });
    if (device) {
      // Check if a link exists for the device with the role of owner. If not, create a new link
      const userDeviceLink = await DBUserDeviceLinks.findOne({ deviceId , role: "owner"});
      if(userDeviceLink) {
        return res.status(400).json({ message: "Device is already logged in." });
      }
      await DBUserDeviceLinks.create({
        deviceId,
        userId: otpData.userId,
        role: "owner"
      });
      return res.status(200).json({ message: "OTP verified"});
    } 
    // Create a new device in the database and a new link for the user
    await DBDevice.create({_id: deviceId, status: "online"});
    await DBUserDeviceLinks.create({
      deviceId,
      userId: otpData.userId,
      role: "owner"
    });
    return res.status(200).json({ message: "OTP verified"});
  } catch (err) {
    console.error('Error:', err);
    return res.status(500).json({ message: "Internal server error" });
  }
};

export const verifyPassword = async (req, res) => {
  try {
    const { email, password, deviceId } = req.body;
    const users = await clerkClient.users.getUserList({ emailAddress: [email] });
    if (users.length === 0 || !users.data || !users.data[0]) {
      return res.status(404).json({ message: "User not found" });
    }
    const userId = users.data[0].id;
    const response = await clerkClient.users.verifyPassword({
      userId,
      password,
    })
    if (response.verified) {
      // Check if the device id is already in the database
      const device = await DBDevice.findOne({ _id: deviceId });
      if (device) {
        // Check if a link exists for the device with the role of owner. If not, create a new link
        const userDeviceLink = await DBUserDeviceLinks.findOne({ deviceId , role: "owner"});
        if(userDeviceLink) {
          return res.status(400).json({ message: "Device is already logged in." });
        }
        await DBUserDeviceLinks.create({
          deviceId,
          userId: userId,
          role: "owner"
        });
        return res.status(200).json({ message: "OTP verified"});
      } 
      // Create a new device in the database and a new link for the user
      await DBDevice.create({_id: deviceId, status: "online"});
      await DBUserDeviceLinks.create({
        deviceId,
        userId,
        role: "owner"
      });
      return res.status(200).json({ message: "OTP verified"});

    } else {
        return res.status(400).json({ message: "Invalid password" });
    }
  } catch (err) {
    console.error('Error:', err);
    if (err.errors[0].code === "incorrect_password") {
      return res.status(400).json({ message: "Incorrect password" });
    }
    return res.status(500).json({ message: "Internal server error" });
  }
};

export const getUserData = async (req, res) => {
  // This function fetches the owner data associated with a device
  try {
    const { deviceId } = req.body;
    const device = await DBDevice.findOne({ _id: deviceId });
    if (!device) {
      return res.status(404).json({ type: "deleted" , message: "Device not found" });
    }
    const userDeviceLink = await DBUserDeviceLinks.findOne({ deviceId, role: "owner" });
    if(!userDeviceLink) {
      return res.status(400).json({ message: "Device isn't logged in" });
    }
    const user = await DBUser.findById(userDeviceLink.userId);
    if (!user) {
      return res.status(404).json({ message: "User not found" });
    }

    const sortedPermissions = Object.fromEntries(Object.entries(userDeviceLink.permissions.toObject()).sort(([permission1, val1], [permission2, val2]) => {
      if (permission1 === "remoteControl") return -10;
      if (permission2 === "remoteControl") return 10;
      if (val1.allowed) return -1;
      if (val2.allowed) return 1;
      return 0;
    }))

    return res.status(200).json({ message: "User data fetched", data: {
      name: (!user.firstName && !user.lastName) ? user.email : (user.firstName || "") + " " + (user.lastName || ""),
      email: user.email,
      profileImage: user.imageUrl,
      permissions: sortedPermissions,
    } });
  } catch (err) {
    console.error('Error:', err);
    return res.status(500).json({ message: "Internal server error" });
  }
}

export const savePermissions = async (req, res) => {
  try {
    const { deviceId, permissions } = req.body;
    const device = await DBDevice.findOne({ _id: deviceId });
    if (!device) {
      return res.status(404).json({ message: "Device not found" });
    }
    const userDeviceLink = await DBUserDeviceLinks.findOne({ deviceId, role: "owner" });
    if(!userDeviceLink) {
      return res.status(400).json({ message: "Login as owner to modify permissions" });
    }
    
    const currentPermissions = userDeviceLink.permissions?.toObject?.() || userDeviceLink.permissions || {};
    
    for (const [permission, value] of Object.entries(permissions)) {
        currentPermissions[permission].allowed = value;
    }
    
    
    await DBUserDeviceLinks.findByIdAndUpdate(userDeviceLink._id, { 
      permissions: currentPermissions 
    });

    const userLinks = await DBUserDeviceLinks.find({ deviceId, role: "user" });
    for (const userLink of userLinks) {
      const userPermissions = userLink.permissions?.toObject?.() || userLink.permissions || {};
      
      for (const [permission, value] of Object.entries(userPermissions)) {
        userPermissions[permission].allowed = 
          value.allowed && currentPermissions[permission].allowed;
      }
      
      await DBUserDeviceLinks.findByIdAndUpdate(userLink._id, { permissions: userPermissions });
    }

    const deviceSessions = await DBSessions.find({ 
                            deviceId, 
                            terminated: false, 
                            $or: [
                              { expiry: { $gt: new Date() } },
                              { expiry: { $eq: null } }
                            ]
                          });
    for (const session of deviceSessions) {
      const sessionPermissions = session.permissions;
      
      for (let i = 0; i < sessionPermissions.length; i++) {
        const permission = sessionPermissions[i];
        const [key, value] = Object.entries(permission)[0];
        
        sessionPermissions[i] = { 
          [key]: value && currentPermissions[key].allowed 
        };
      }
      
      await DBSessions.findByIdAndUpdate(session._id, { 
        permissions: sessionPermissions 
      });
    }


    // Instantaneously updating permissions for the active user and device
    let permissionsSend = { permissions: Object.fromEntries(
              Object.entries(currentPermissions).map(([key, value]) => [key, value.allowed])
          )
        }
    
    const activeLink = await DBUserDeviceLinks.findOne({ deviceId, active: true });
    if (activeLink && activeLink.sessionKey) {
      const session = await DBSessions.findOne({ _id: activeLink.sessionKey });
      const sessionPermissions = session.permissions;
      for (const permission of sessionPermissions) {
        const [key, value] = Object.entries(permission)[0];
        permissionsSend.permissions[key] = value && currentPermissions[key].allowed;
      }
    }
    if (activeLink && userDeviceManager.areConnected(activeLink.userId, deviceId)) {
      io.to(userDeviceManager.getUserSocketIdByDeviceId(deviceId)).emit("permissions", permissionsSend);
      io.to(userDeviceManager.getDeviceSocketIdByUserId(activeLink.userId)).emit("permissions", permissionsSend);
    }
    return res.status(200).json({ message: "Permissions saved" });
  } catch (err) {
    console.error('Error:', err);
    return res.status(500).json({ message: "Internal server error" });
  }
}

export const enableDisable = async (req, res) => {
  try {
    const { deviceId, linked } = req.body;
    await DBDevice.findByIdAndUpdate(deviceId, {linked});
    return res.status(200).json({ message: linked ? "Enabled Control" : "Disabled Control" });
  } catch (err) {
    console.error('Error:', err);
    return res.status(500).json({ message: "Internal server error" });
  }
}

export const updateDeviceInfo = async (req, res) => {
  try {
    const { deviceId, deviceInfo } = req.body;
    await DBDevice.findByIdAndUpdate(deviceId, {deviceData: deviceInfo})
    return res.status(200).json({ message: "Device info updated" });
  } catch (err) {
    console.error('Error:', err);
    return res.status(500).json({ message: "Internal server error" });
  }
}

export const logoutApp = async (req, res) => {
  try {
    const { deviceId} = req.body;
    const device = await DBDevice.findOne({ _id: deviceId });
    if (device) {
      const userDeviceLink = await DBUserDeviceLinks.findOne({ deviceId, role: "owner" });
      if (!userDeviceLink) {
        return res.status(400).json({ message: "Device is not logged in" });
      }
      // [MUTANT GS_M54: SDL] deleteMany deleted in logoutApp
      return res.status(200).json({ message: "Device logged out" });
    }
    return res.status(404).json({ message: "Device not found" });
  } catch (err) {
    console.error('Error:', err);
    return res.status(500).json({ message: "Internal server error" });
  }
}