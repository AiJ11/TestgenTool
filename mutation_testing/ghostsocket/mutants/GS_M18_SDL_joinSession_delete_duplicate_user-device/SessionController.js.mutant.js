import DBSessions from "../models/SessionModel.js";
import DBUserDeviceLinks from "../models/UserDeviceLinksModel.js";
import DBDevice from "../models/DevicesModel.js";
import DBUser from "../models/UserModel.js";
import { io, userDeviceManager } from "../socket.js";

const formatDate = (date) => {
    const options = { year: 'numeric', month: 'long', day: 'numeric', hour: '2-digit', minute: '2-digit' };
    return date.toLocaleDateString('en-US', options);
}


export async function createSession(req, res) {
    try {
        const { deviceId, expiry, permissions } = req.body;
        const userId = req.auth.userId;

        //if there exists expiry check if it is atlesast 10 minutes from now
        if (expiry) {
            const currentDate = new Date();
            const selectedDate = new Date(expiry);
            const tenMinutesFromNow = new Date(currentDate.getTime() + 10 * 60 * 1000);

            if (selectedDate < tenMinutesFromNow) {
                return res.status(400).json({ error: "Expiry must be at least 10 minutes from now." });
            }
        }

        // Check if permissions are provided else return error
        if (!permissions || !Array.isArray(permissions) || permissions.length === 0) {
            return res.status(400).json({ error: "Permissions are required." });
        }

        // Check if deviceId and UserId Link Exists and role is owner
        const userDeviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId, role: "owner" });
        if (!userDeviceLink) {
            return res.status(403).json({ error: "You do not have permission to create a session for this device." });
        }

        // Create session and get Id
        const session = await DBSessions.create({
            deviceId,
            userId,
            expiry: expiry ? new Date(expiry) : null,
            permissions: permissions,
        });

        res.status(201).json({ sessionKey: session._id });
    } catch (error) {
        console.error("Error creating session:", error);
        res.status(500).json({ error: "Failed to create session" });
    }
}

export async function joinSession(req, res) {
    try {
        const { sessionKey } = req.body;
        const userId = req.auth.userId;

        // Validate sessionKey format
        if (!sessionKey || typeof sessionKey !== 'string' || sessionKey.length !== 24) {
            return res.status(400).json({ message: "Invalid session key format." });
        }

        // check if the session exists and is not expired
        const session = await DBSessions.findOne({ _id: sessionKey, joinedUserId: null, terminated: false });
        if (!session) {
            return res.status(404).json({ message: "Session not found or used." });
        }
        // Check if the session has expired if its null then it means manual expiry
        if (session.expiry && new Date(session.expiry) < new Date()) {
            return res.status(400).json({ message: "Session has expired." });
        }

        // Check if the device exists in the DBDevices collection from the deviceId you got from te session
        const device = await DBDevice.findOne({ _id: session.deviceId });
        if (!device) {
            return res.status(404).json({ message: "Device not found." });
        }

        // [MUTANT GS_M18: SDL] duplicate-link check deleted

        // Create a new user device link for the user
        await DBUserDeviceLinks.create({
            userId,
            deviceId: session.deviceId,
            role: "user",
            expiry: session.expiry,
            sessionKey: session._id,
            permissions: session.permissions.reduce((acc, perm) => {
                const key = Object.keys(perm)[0];
                acc[key] = { allowed: perm[key] };
                return acc;
            }, {})
        });

        // Mark the session as used and update the joinedUserId
        await DBSessions.updateOne({ _id: sessionKey }, {
            $set: { joinedUserId: userId, accessedDate: new Date() },
        });
        res.status(200).json({ message: "Joined Session Sucessfully.", deviceId: session.deviceId });
    } catch (error) {
        console.error("Error joining session:", error);
        res.status(500).json({ error: "Failed to join session" });
    }
}

export async function getSessions(req, res) {
    try {
        const userId = req.auth.userId;
        // Fetch all of users created sessions
        const createdSessions = await DBSessions.find({ userId }).sort({ createdAt: -1 });
        const formattedCreatedSessions = await Promise.all(createdSessions.map(async session => {
            const deviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId: session.deviceId });
            let devStatus;
            if (session.terminated) {
                devStatus = "terminated"; // Session is terminated
            } else if (session.joinedUserId && session.expiry && session.expiry < new Date() || session.terminated) {
                devStatus = "completed"; // Joined the session and expired
            }
            else if (session.expiry && session.expiry < new Date()) {
                devStatus = "expired"; // Not joined the session and expired
            } else if (session.joinedUserId) {
                devStatus = "active"; // Joined the session and not expired
            } else {
                devStatus = "pending"; // Not joined the session and not expired
            }
            const joinedUser = session.joinedUserId ? await DBUser.findOne({ _id: session.joinedUserId }) : null;
            let joinedUserName = "-";
            if (joinedUser) {
                joinedUserName = !joinedUser.firstName && !joinedUser.lastName ?
                    joinedUser.email :
                    (joinedUser.firstName ? joinedUser.firstName + " " : "") + (joinedUser.lastName ? joinedUser.lastName : "");
            }
            return ({
                sessionKey: session._id,
                deviceName: deviceLink.name,
                status: devStatus,
                type: "created",
                createdAt: formatDate(session.createdAt),
                joinedUserName: joinedUserName,
                expiry: session.expiry ? formatDate(new Date(session.expiry)) : "Manual Expiry",
                permissions: session.permissions,
            })
        }))

        const joinedSessions = await DBSessions.find({ joinedUserId: userId }).sort({ createdAt: -1 });
        const formattedJoinedSessions = await Promise.all(joinedSessions.map(async (session) => {
            const deviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId: session.deviceId });
            let devStatus;
            // We definetely know joinedUserId is not null here
            if (session.terminated) {
                devStatus = "terminated"; // Session is terminated
            } else if (session.expiry && session.expiry < new Date()) {
                devStatus = "completed"; // Joined the session and expired
            } else {
                devStatus = "active"; // Joined the session and not expired
            }
            const createdUser = await DBUser.findOne({ _id: session.userId });
            const createdUserName = !createdUser.firstName && !createdUser.lastName ?
                createdUser.email :
                (createdUser.firstName ? createdUser.firstName + " " : "") + (createdUser.lastName ? createdUser.lastName : "");
            return ({
                sessionKey: session._id,
                deviceName: deviceLink ? deviceLink.name : session.deviceId,
                status: devStatus,
                type: "joined",
                createdAt: formatDate(session.createdAt),
                createdUserName: createdUserName,
                expiry: session.expiry ? formatDate(new Date(session.expiry)) : "Manual Expiry",
                permissions: session.permissions
            })
        }));

        res.status(200).json([...formattedCreatedSessions, ...formattedJoinedSessions]);
    } catch (error) {
        console.error("Error fetching sessions:", error);
        res.status(500).json({ error: "Failed to fetch sessions" });
    }
}

export async function connectedSessions(req, res) {
    // Active, Completed, Terminated
    const userId = req.auth.userId;
    const { deviceId } = req.params;
    // get all the sessions that have joineduserid not null and have userid as userId
    try {
        const userDeviceLinks = await DBUserDeviceLinks.find({ userId, deviceId, role: "owner" });
        if (userDeviceLinks.length === 0) {
            return res.status(200).json([]);
        }
        const connectedSessions = await DBSessions.find(
            { userId: userId, deviceId, joinedUserId: { $ne: null } }).sort({ updatedAt: -1 });
        const formattedSessions = await Promise.all(connectedSessions.map(async (session) => {
            let sessionStatus;
            if (session.terminated) {
                sessionStatus = "terminated"; // Session is terminated
            }
            else if (session.expiry && session.expiry < new Date()) {
                sessionStatus = "completed"; // Joined the session and expired
            } else {
                sessionStatus = "active"; // Joined the session and not expired
            }
            const joinedUser = await DBUser.findOne({ _id: session.joinedUserId });
            const joinedUserName = !joinedUser.firstName && !joinedUser.lastName ?
                joinedUser.email :
                (joinedUser.firstName ? joinedUser.firstName + " " : "") + (joinedUser.lastName ? joinedUser.lastName : "");
            return ({
                sessionKey: session._id,
                joinedUserName: joinedUserName,
                accessedDate: formatDate(session.accessedDate),
                permissions: session.permissions,
                status: sessionStatus,
            });
        }));
        res.status(200).json(formattedSessions);
    } catch (error) {
        console.error("Error fetching connected sessions:", error);
        return res.status(500).json({ error: "Failed to fetch connected sessions" });
    }
}

export async function terminateSession(req, res) {
    const { sessionKey } = req.params;
    const userId = req.auth.userId;

    try {
        // Check if the session exists and is not expired
        const session = await DBSessions.findOne({ _id: sessionKey });
        if (!session || session.terminated) {
            return res.status(404).json({ message: "Session not found or already terminated." });
        }
        // Check if the user is the owner of the session
        if (session.userId !== userId) {
            return res.status(403).json({ message: "You are not authorized to terminate this session." });
        }
        // Update the session to mark it as terminated
        await DBSessions.updateOne({ _id: sessionKey }, { $set: { terminated: true } });
        // If the session has a joinedUserId, we also need to terminate their session
        const activeDeviceLink = await DBUserDeviceLinks.findOne({ sessionKey, active: true });
        await DBUserDeviceLinks.deleteOne({ sessionKey });

        if (activeDeviceLink) {
            const userSocketId = userDeviceManager.getUserSocketIdByDeviceId(activeDeviceLink.deviceId);
            if (userSocketId) {
                io.to(userSocketId).emit("sessionTerminated", { sessionKey });
            }
        }

        res.status(200).json({ message: "Session terminated successfully." });
    } catch (error) {
        console.error("Error terminating session:", error);
        res.status(500).json({ error: "Failed to terminate session" });
    }
}

export async function dropAllSessions(req, res) {

}

export async function updatePermissions(req, res) {
    const { sessionKey, permissions } = req.body;
    const userId = req.auth.userId;

    try {
        // Check if the session exists and is not expired
        const session = await DBSessions.findOne({ _id: sessionKey, userId });
        if (!session || session.terminated) {
            return res.status(404).json({ message: "Session not found or already terminated." });
        }

        // Validate permissions format
        if (!Array.isArray(permissions) || permissions.length === 0) {
            return res.status(400).json({ message: "Invalid permissions format." });
        }

        // Update the session with new permissions
        const updatedSession = await DBSessions.updateOne(
            { _id: sessionKey },
            { $set: { permissions } }
        );

        // check if userdevice link exists and update the user device link permissions
        const userDeviceLink = await DBUserDeviceLinks.findOne({ sessionKey });
        let updatedPermissions;
        if (userDeviceLink) {
            updatedPermissions = permissions.reduce((acc, perm) => {
                const key = Object.keys(perm)[0];
                acc[key] = { allowed: perm[key] };
                return acc;
            }, {});
            await DBUserDeviceLinks.updateOne(
                { sessionKey },
                { $set: { permissions: updatedPermissions } }
            );
        }

        // Instantaneously update the permissions on the device and user if they are connected
        if (session.joinedUserId && userDeviceManager.areConnected(session.joinedUserId, session.deviceId)) {
            const permissionsToSend = { permissions: Object.fromEntries(Object.entries(updatedPermissions).map(([key, value]) => [key, value.allowed])) }
            const deviceSocketId = userDeviceManager.getDeviceSocketIdByUserId(session.joinedUserId);
            const userSocketId = userDeviceManager.getUserSocketIdByDeviceId(session.deviceId);

            io.to(deviceSocketId).emit("permissions", permissionsToSend);
            io.to(userSocketId).emit("permissions", permissionsToSend);
        }
        res.status(200).json({ message: "Permissions updated successfully." });
    } catch (error) {
        console.error("Error updating permissions:", error);
        res.status(500).json({ error: "Failed to update permissions" });
    }
}