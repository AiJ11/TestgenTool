import DBDevice from "../models/DevicesModel.js";
import DBUserDeviceLinks from "../models/UserDeviceLinksModel.js";
import DBUploads from "../models/UploadsModel.js";
import DBSessions from "../models/SessionModel.js";


export const getMyDevices = async (req, res) => {
  try {
    const { userId } = req.auth;
    const myDevices = await DBUserDeviceLinks.find({ userId, role: "owner" })
      .populate({
        path: 'deviceId',
        model: 'Devices',
        select: 'inUse deviceData status'
      });
    
    const devicesWithInfo = myDevices.map(link => ({
      _id: link._id,
      deviceId: link.deviceId._id,
      name: link.name,
      os: link.deviceId.deviceData.os,
      ip: link.deviceId.deviceData.ip,
      location: link.deviceId.deviceData.location,
      status: link.deviceId.status,
      inUse: link.deviceId.inUse,
      accessLevel: link.deviceId.deviceData.role,
      role: link.role,
      active: link.active,
    }));
    res.status(200).json(devicesWithInfo);
  } catch (error) {
    console.error("Error fetching my devices:", error);
    res.status(500).json({ error: "Failed to fetch devices" });
  }
}

export const getOtherDevices = async (req, res) => {
  try {
    const { userId } = req.auth;
    const myDevices = await DBUserDeviceLinks.find({ userId, role: "user" })
      .populate({
        path: 'deviceId',
        model: 'Devices',
        select: 'inUse deviceData status'
      });
    
    const devicesWithInfo = myDevices.map(link => ({
      _id: link._id,
      deviceId: link.deviceId._id,
      name: link.name,
      os: link.deviceId.deviceData.os,
      ip: link.deviceId.deviceData.ip,
      location: link.deviceId.deviceData.location,
      status: link.deviceId.status,
      inUse: link.deviceId.inUse,
      accessLevel: link.deviceId.deviceData.role,
      role: link.role,
      active: link.active,
    }));
    res.status(200).json(devicesWithInfo);
  } catch (error) {
    console.error("Error fetching my devices:", error);
    res.status(500).json({ error: "Failed to fetch devices" });
  }
}

export const updateName = async (req, res) => {
  const { linkId } = req.params;
  const { name } = req.body;
  try {
    await DBUserDeviceLinks.updateOne({ _id: linkId }, { $set: { name } });
    res.status(200).json({ message: "Device name updated successfully" });
  } catch (error) {
    console.error("Error updating device name:", error);
    res.status(500).json({ error: "Failed to update device name" });
  }
}

export const getDeviceInfo = async (req, res) => {
  const { deviceId } = req.params;
  const { userId } = req.auth;
  try {
    // Check if the user has access to the device
    const userDeviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId });
    if (!userDeviceLink) {
      return res.status(403).json({ error: "You do not have access to this device" });
    }
    // Fetch the device information
    if (!deviceId) {
      return res.status(400).json({ error: "Device ID is required" });
    }
    
    const device = await DBDevice.findById(deviceId)
    const deviceInfo = device.toObject().deviceData;
    const permissions = Object.entries(userDeviceLink.permissions.toObject() || {}).map(([key, value]) => ({
      key,
      value
    }));

    res.status(200).json({deviceInfo: {
      name: userDeviceLink.name || "Unnamed Device",
      linkRole: userDeviceLink.role,
      deviceId: device._id,
      ...deviceInfo,
      status: device.status,
    }, permissions});
  } catch (error) {
    console.error("Error fetching device info:", error);
    res.status(500).json({ error: "Failed to fetch device info" });
  }
}

export const uplodFile = async (req, res) => {
  try {
    if (!req.file) {
      return res.status(400).json({ error: "No file uploaded" });
    }
    await DBUploads.create({
      imageId: req.file.filename,
      userId: req.auth.userId,
      deviceId: req.params.deviceId,
      type: req.body.type
    })
    res.json({ id: req.file.filename });
  } catch (error) {
    console.error("Error uploading file:", error);
    res.status(500).json({ error: "Failed to upload file" });
  }
}

export const getUploads = async (req, res) => {
  try{
    if (!req.query.type) {
      return res.status(400).json({ error: "type query parameters is required" });
    }
    const uploads = await DBUploads.find(
      { userId: req.auth.userId, deviceId: req.params.deviceId, type: req.query.type },
      { imageId: 1, _id: 0 }
    ).sort({ createdAt: -1 });
    const imageIds = uploads.map(upload => upload.imageId);
    res.json(imageIds);
  } catch (error) {
    console.error("Error fetching uploads:", error);
    res.status(500).json({ error: "Failed to fetch uploads" });
  }
}

export const deleteDevice = async (req, res) => {
  const { deviceId } = req.params;
  const { userId } = req.auth;
  try {
    // Check if the user has access to the device
    const userDeviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId});
    if (!userDeviceLink) {
      return res.status(403).json({ error: "You do not have access to this device" });
    }
    if (userDeviceLink.role === "owner") {
      // If the user is the owner, delete the device and all associated links
      await DBUserDeviceLinks.deleteMany({ deviceId });
      await DBDevice.deleteOne({ _id: deviceId });
      await DBSessions.updateMany({ deviceId }, { $set: { terminated: true } });
      res.status(200).json({ message: "Device deleted successfully" });
    } else {
      // If the user is not the owner, just delete the link
      await DBUserDeviceLinks.deleteOne({ _id: userDeviceLink._id });
      await DBSessions.updateMany({ deviceId, joinedUserId: userId }, { $set: { terminated: true } });
      res.status(201).json({ message: "Device link deleted successfully" });
    }
  } catch (error) {
    console.error("Error deleting device:", error);
    res.status(500).json({ error: "Failed to delete device" });
  }
}