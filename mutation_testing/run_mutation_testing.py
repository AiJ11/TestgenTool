#!/usr/bin/env python3
"""
Mutation Testing Framework for TestGen Tool
============================================
Applies standard mutation operators to GhostSocket backend controllers
and evaluates how many mutants TestGen's test suite can kill.

Standard Mutation Operators Used:
  ROR  - Relational Operator Replacement
  LCR  - Logical Connector Replacement
  AOR  - Arithmetic Operator Replacement
  CDL  - Constant/Literal Replacement
  SDL  - Statement Deletion
  UOI  - Unary Operator Insertion
  BCR  - Boolean Condition Replacement

Usage:
  python3 run_mutation_testing.py [--apply] [--report] [--run-testgen]

  --apply       : Create mutant files in ./mutants/ directory
  --report      : Print full mutation testing report
  --run-testgen : Run TestGen against each mutant (requires backend running)
"""

import os
import shutil
import subprocess
import argparse
from dataclasses import dataclass
from typing import Optional

# ─────────────────────────────────────────────────────────────────────────────
# Configuration
# ─────────────────────────────────────────────────────────────────────────────

BACKEND_ROOT  = "/Users/nakulpanwar/Desktop/GhostSocket/server"
TESTGEN_ROOT  = "/Users/nakulpanwar/Desktop/TestgenTool"
TESTGEN_BIN   = f"{TESTGEN_ROOT}/test_libapplication"
MUTANTS_DIR   = "./mutants"

# TestGen test IDs that exercise GhostSocket endpoints
TESTGEN_TESTS = [
    "test01_registerUser",
    "test02_registerTwoUsers",
    "test03_registerDevice",
    "test04_getMyDevices",
    "test05_getDeviceInfo",
    "test06_createSession",
    "test07_joinSession",
    "test08_getSessions",
    "test09_terminateSession",
    "test10_deleteDevice",
    "test11_getOtherDevices",
    "test12_updatePermissions",
    "test13_deviceInfoForbidden",
    "test14_createSessionForbidden",
    "test15_joinSessionNotFound",
    "test16_terminateSessionForbidden",
    "test17_fullSessionLifecycle",
    "test18_devicesAndSession",
    "test19_deviceInfoAndSession",
    "test20_joinAndUpdatePermissions",
    "test21_sessionListAndTerminate",
    "test22_createSessionNoDevice",
    "test23_joinSessionNoSession",
    "test24_terminateNoSession",
    "test25_getDeviceInfoNoDevice",
]

# ─────────────────────────────────────────────────────────────────────────────
# Mutation dataclass
# ─────────────────────────────────────────────────────────────────────────────

@dataclass
class Mutation:
    id: str                    # e.g. "M01"
    operator: str              # e.g. "CDL", "ROR", "SDL", "LCR", "AOR", "UOI", "BCR"
    file: str                  # relative path from BACKEND_ROOT
    original: str              # exact text to replace
    mutant: str                # replacement text
    description: str           # human-readable description
    killing_tests: list        # TestGen tests expected to kill this mutant
    killed: Optional[bool] = None  # filled in after running TestGen

    @property
    def source_path(self):
        return os.path.join(BACKEND_ROOT, self.file)

    @property
    def mutant_dir(self):
        return os.path.join(MUTANTS_DIR, self.id)

    @property
    def mutant_file(self):
        return os.path.join(self.mutant_dir, os.path.basename(self.file))

# ─────────────────────────────────────────────────────────────────────────────
# Mutation catalogue  (54 mutations across 3 controllers)
# ─────────────────────────────────────────────────────────────────────────────

MUTATIONS = [

    # ═══════════════════════════════════════════════════════════════════
    # SESSION CONTROLLER  (controllers/SessionController.js)
    # ═══════════════════════════════════════════════════════════════════

    # ── createSession ──────────────────────────────────────────────────
    Mutation(
        id="M01", operator="CDL",
        file="controllers/SessionController.js",
        original="res.status(201).json({ sessionKey: session._id });",
        mutant="res.status(200).json({ sessionKey: session._id });",
        description="[CDL] createSession: status 201 → 200 (wrong success code)",
        killing_tests=["test06_createSession","test07_joinSession","test08_getSessions",
                       "test09_terminateSession","test12_updatePermissions",
                       "test17_fullSessionLifecycle","test18_devicesAndSession",
                       "test19_deviceInfoAndSession","test20_joinAndUpdatePermissions",
                       "test21_sessionListAndTerminate"],
    ),
    Mutation(
        id="M02", operator="CDL",
        file="controllers/SessionController.js",
        original='return res.status(403).json({ error: "You do not have permission to create a session for this device." });',
        mutant='return res.status(200).json({ error: "You do not have permission to create a session for this device." });',
        description="[CDL] createSession: ownership-check 403 → 200 (auth bypass)",
        killing_tests=["test14_createSessionForbidden"],
    ),
    Mutation(
        id="M03", operator="UOI",
        file="controllers/SessionController.js",
        original='const userDeviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId, role: "owner" });\n        if (!userDeviceLink) {',
        mutant='const userDeviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId, role: "owner" });\n        if (userDeviceLink) {',
        description="[UOI] createSession: negate ownership guard (!userDeviceLink → userDeviceLink)",
        killing_tests=["test06_createSession","test14_createSessionForbidden"],
    ),
    Mutation(
        id="M04", operator="CDL",
        file="controllers/SessionController.js",
        original='const userDeviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId, role: "owner" });',
        mutant='const userDeviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId, role: "user" });',
        description="[CDL] createSession: role filter 'owner' → 'user' (wrong role check)",
        killing_tests=["test06_createSession","test14_createSessionForbidden"],
    ),
    Mutation(
        id="M05", operator="ROR",
        file="controllers/SessionController.js",
        original="if (selectedDate < tenMinutesFromNow) {",
        mutant="if (selectedDate > tenMinutesFromNow) {",
        description="[ROR] createSession: expiry guard < → > (accepts sessions expiring soon)",
        killing_tests=["test06_createSession","test07_joinSession","test08_getSessions"],
    ),
    Mutation(
        id="M06", operator="ROR",
        file="controllers/SessionController.js",
        original="if (selectedDate < tenMinutesFromNow) {",
        mutant="if (selectedDate <= tenMinutesFromNow) {",
        description="[ROR] createSession: expiry guard < → <= (off-by-one at boundary)",
        killing_tests=["test06_createSession"],
    ),
    Mutation(
        id="M07", operator="AOR",
        file="controllers/SessionController.js",
        original="const tenMinutesFromNow = new Date(currentDate.getTime() + 10 * 60 * 1000);",
        mutant="const tenMinutesFromNow = new Date(currentDate.getTime() - 10 * 60 * 1000);",
        description="[AOR] createSession: expiry threshold + → - (always rejects valid sessions)",
        killing_tests=["test06_createSession","test07_joinSession","test09_terminateSession"],
    ),
    Mutation(
        id="M08", operator="AOR",
        file="controllers/SessionController.js",
        original="const tenMinutesFromNow = new Date(currentDate.getTime() + 10 * 60 * 1000);",
        mutant="const tenMinutesFromNow = new Date(currentDate.getTime() + 9 * 60 * 1000);",
        description="[AOR] createSession: 10 min threshold → 9 min (weakened validation)",
        killing_tests=["test06_createSession"],
    ),
    Mutation(
        id="M09", operator="ROR",
        file="controllers/SessionController.js",
        original="if (!permissions || !Array.isArray(permissions) || permissions.length === 0) {",
        mutant="if (!permissions || !Array.isArray(permissions) || permissions.length !== 0) {",
        description="[ROR] createSession: permissions.length === 0 → !== 0 (rejects non-empty arrays)",
        killing_tests=["test06_createSession","test07_joinSession","test12_updatePermissions"],
    ),
    Mutation(
        id="M10", operator="LCR",
        file="controllers/SessionController.js",
        original="if (!permissions || !Array.isArray(permissions) || permissions.length === 0) {",
        mutant="if (!permissions && !Array.isArray(permissions) && permissions.length === 0) {",
        description="[LCR] createSession: || → && in permissions guard (bypasses validation)",
        killing_tests=["test06_createSession"],
    ),
    Mutation(
        id="M11", operator="SDL",
        file="controllers/SessionController.js",
        original="""        // Check if permissions are provided else return error
        if (!permissions || !Array.isArray(permissions) || permissions.length === 0) {
            return res.status(400).json({ error: "Permissions are required." });
        }""",
        mutant="        // [MUTANT M11: SDL] permissions check deleted",
        description="[SDL] createSession: delete permissions validation",
        killing_tests=["test06_createSession"],
    ),
    Mutation(
        id="M12", operator="CDL",
        file="controllers/SessionController.js",
        original='return res.status(400).json({ error: "Expiry must be at least 10 minutes from now." });',
        mutant='return res.status(200).json({ error: "Expiry must be at least 10 minutes from now." });',
        description="[CDL] createSession: expiry error 400 → 200",
        killing_tests=["test06_createSession"],
    ),

    # ── joinSession ────────────────────────────────────────────────────
    Mutation(
        id="M13", operator="CDL",
        file="controllers/SessionController.js",
        original="const session = await DBSessions.findOne({ _id: sessionKey, joinedUserId: null, terminated: false });",
        mutant="const session = await DBSessions.findOne({ _id: sessionKey, joinedUserId: null, terminated: true });",
        description="[CDL] joinSession: terminated: false → true (only finds terminated sessions)",
        killing_tests=["test07_joinSession","test11_getOtherDevices","test17_fullSessionLifecycle",
                       "test20_joinAndUpdatePermissions"],
    ),
    Mutation(
        id="M14", operator="SDL",
        file="controllers/SessionController.js",
        original="const session = await DBSessions.findOne({ _id: sessionKey, joinedUserId: null, terminated: false });",
        mutant="const session = await DBSessions.findOne({ _id: sessionKey });",
        description="[SDL] joinSession: remove joinedUserId/terminated conditions (allows re-joining used sessions)",
        killing_tests=["test07_joinSession","test17_fullSessionLifecycle"],
    ),
    Mutation(
        id="M15", operator="CDL",
        file="controllers/SessionController.js",
        original='return res.status(404).json({ message: "Session not found or used." });',
        mutant='return res.status(200).json({ message: "Session not found or used." });',
        description="[CDL] joinSession: 404 → 200 on session-not-found",
        killing_tests=["test15_joinSessionNotFound"],
    ),
    Mutation(
        id="M16", operator="ROR",
        file="controllers/SessionController.js",
        original="if (session.expiry && new Date(session.expiry) < new Date()) {",
        mutant="if (session.expiry && new Date(session.expiry) > new Date()) {",
        description="[ROR] joinSession: expiry check < → > (rejects valid sessions, allows expired)",
        killing_tests=["test07_joinSession","test11_getOtherDevices","test17_fullSessionLifecycle"],
    ),
    Mutation(
        id="M17", operator="SDL",
        file="controllers/SessionController.js",
        original="""        // Check if the session has expired if its null then it means manual expiry
        if (session.expiry && new Date(session.expiry) < new Date()) {
            return res.status(400).json({ message: "Session has expired." });
        }""",
        mutant="        // [MUTANT M17: SDL] expiry check deleted",
        description="[SDL] joinSession: delete expiry validation (allows joining expired sessions)",
        killing_tests=["test07_joinSession"],
    ),
    Mutation(
        id="M18", operator="SDL",
        file="controllers/SessionController.js",
        original="""        // Check if the user device link already exists
        const userDeviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId: session.deviceId });
        if (userDeviceLink) {
            return res.status(400).json({ message: " connected to this device." });
        }""",
        mutant="        // [MUTANT M18: SDL] duplicate-link check deleted",
        description="[SDL] joinSession: delete duplicate user-device-link check",
        killing_tests=["test11_getOtherDevices"],
    ),

    # ── terminateSession ───────────────────────────────────────────────
    Mutation(
        id="M19", operator="ROR",
        file="controllers/SessionController.js",
        original="if (session.userId !== userId) {",
        mutant="if (session.userId === userId) {",
        description="[ROR] terminateSession: !== → === in owner check (only owner cannot terminate)",
        killing_tests=["test09_terminateSession","test16_terminateSessionForbidden",
                       "test17_fullSessionLifecycle","test21_sessionListAndTerminate"],
    ),
    Mutation(
        id="M20", operator="UOI",
        file="controllers/SessionController.js",
        original="if (session.userId !== userId) {",
        mutant="if (!(session.userId !== userId)) {",
        description="[UOI] terminateSession: double-negation inserted on owner check",
        killing_tests=["test09_terminateSession","test16_terminateSessionForbidden"],
    ),
    Mutation(
        id="M21", operator="CDL",
        file="controllers/SessionController.js",
        original='return res.status(403).json({ message: "You are not authorized to terminate this session." });',
        mutant='return res.status(200).json({ message: "You are not authorized to terminate this session." });',
        description="[CDL] terminateSession: 403 → 200 on unauthorized terminate",
        killing_tests=["test16_terminateSessionForbidden"],
    ),
    Mutation(
        id="M22", operator="SDL",
        file="controllers/SessionController.js",
        original="""        // Check if the user is the owner of the session
        if (session.userId !== userId) {
            return res.status(403).json({ message: "You are not authorized to terminate this session." });
        }""",
        mutant="        // [MUTANT M22: SDL] ownership check for terminate deleted",
        description="[SDL] terminateSession: delete userId ownership check (anyone can terminate)",
        killing_tests=["test09_terminateSession","test16_terminateSessionForbidden",
                       "test17_fullSessionLifecycle"],
    ),
    Mutation(
        id="M23", operator="CDL",
        file="controllers/SessionController.js",
        original="await DBSessions.updateOne({ _id: sessionKey }, { $set: { terminated: true } });",
        mutant="await DBSessions.updateOne({ _id: sessionKey }, { $set: { terminated: false } });",
        description="[CDL] terminateSession: terminated: true → false (session not marked terminated)",
        killing_tests=["test09_terminateSession","test17_fullSessionLifecycle",
                       "test21_sessionListAndTerminate"],
    ),
    Mutation(
        id="M24", operator="LCR",
        file="controllers/SessionController.js",
        original="if (!session || session.terminated) {",
        mutant="if (!session && session.terminated) {",
        description="[LCR] terminateSession: || → && in not-found guard (may crash on null)",
        killing_tests=["test09_terminateSession"],
    ),

    # ── updatePermissions ──────────────────────────────────────────────
    Mutation(
        id="M25", operator="CDL",
        file="controllers/SessionController.js",
        original="const session = await DBSessions.findOne({ _id: sessionKey, userId });",
        mutant="const session = await DBSessions.findOne({ _id: sessionKey });",
        description="[CDL] updatePermissions: remove userId scope (any user can update any session)",
        killing_tests=["test12_updatePermissions","test20_joinAndUpdatePermissions"],
    ),
    Mutation(
        id="M26", operator="SDL",
        file="controllers/SessionController.js",
        original="""        // Validate permissions format
        if (!Array.isArray(permissions) || permissions.length === 0) {
            return res.status(400).json({ message: "Invalid permissions format." });
        }""",
        mutant="        // [MUTANT M26: SDL] permissions format validation deleted",
        description="[SDL] updatePermissions: delete permissions format validation",
        killing_tests=["test12_updatePermissions"],
    ),
    Mutation(
        id="M27", operator="CDL",
        file="controllers/SessionController.js",
        original='        res.status(200).json({ message: "Permissions updated successfully." });',
        mutant='        res.status(201).json({ message: "Permissions updated successfully." });',
        description="[CDL] updatePermissions: success 200 → 201",
        killing_tests=["test12_updatePermissions","test20_joinAndUpdatePermissions"],
    ),

    # ═══════════════════════════════════════════════════════════════════
    # DEVICE CONTROLLER  (controllers/DeviceController.js)
    # ═══════════════════════════════════════════════════════════════════

    # ── getDeviceInfo ──────────────────────────────────────────────────
    Mutation(
        id="M28", operator="UOI",
        file="controllers/DeviceController.js",
        original="    if (!userDeviceLink) {\n      return res.status(403).json({ error: \"You do not have access to this device\" });",
        mutant="    if (userDeviceLink) {\n      return res.status(403).json({ error: \"You do not have access to this device\" });",
        description="[UOI] getDeviceInfo: negate guard (!userDeviceLink → userDeviceLink)",
        killing_tests=["test05_getDeviceInfo","test13_deviceInfoForbidden","test19_deviceInfoAndSession"],
    ),
    Mutation(
        id="M29", operator="CDL",
        file="controllers/DeviceController.js",
        original="      return res.status(403).json({ error: \"You do not have access to this device\" });",
        mutant="      return res.status(200).json({ error: \"You do not have access to this device\" });",
        description="[CDL] getDeviceInfo: 403 → 200 on access denied",
        killing_tests=["test13_deviceInfoForbidden"],
    ),
    Mutation(
        id="M30", operator="SDL",
        file="controllers/DeviceController.js",
        original="""    // Check if the user has access to the device
    const userDeviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId });
    if (!userDeviceLink) {
      return res.status(403).json({ error: "You do not have access to this device" });
    }""",
        mutant="    // [MUTANT M30: SDL] access check deleted",
        description="[SDL] getDeviceInfo: delete access check (any user sees any device)",
        killing_tests=["test13_deviceInfoForbidden"],
    ),
    Mutation(
        id="M31", operator="CDL",
        file="controllers/DeviceController.js",
        original='res.status(200).json({deviceInfo: {',
        mutant='res.status(201).json({deviceInfo: {',
        description="[CDL] getDeviceInfo: success 200 → 201",
        killing_tests=["test05_getDeviceInfo","test19_deviceInfoAndSession"],
    ),

    # ── deleteDevice ───────────────────────────────────────────────────
    Mutation(
        id="M32", operator="CDL",
        file="controllers/DeviceController.js",
        original='    const userDeviceLink = await DBUserDeviceLinks.findOne({ userId, deviceId});',
        mutant='    const userDeviceLink = await DBUserDeviceLinks.findOne({ deviceId});',
        description="[CDL] deleteDevice: remove userId from findOne (any user can delete any device)",
        killing_tests=["test10_deleteDevice"],
    ),
    Mutation(
        id="M33", operator="ROR",
        file="controllers/DeviceController.js",
        original='    if (userDeviceLink.role === "owner") {',
        mutant='    if (userDeviceLink.role !== "owner") {',
        description="[ROR] deleteDevice: role === 'owner' → !== 'owner' (inverted owner check)",
        killing_tests=["test10_deleteDevice"],
    ),
    Mutation(
        id="M34", operator="CDL",
        file="controllers/DeviceController.js",
        original='    if (userDeviceLink.role === "owner") {',
        mutant='    if (userDeviceLink.role === "user") {',
        description="[CDL] deleteDevice: 'owner' → 'user' role check (users do full delete)",
        killing_tests=["test10_deleteDevice"],
    ),
    Mutation(
        id="M35", operator="SDL",
        file="controllers/DeviceController.js",
        original="      await DBDevice.deleteOne({ _id: deviceId });",
        mutant="      // [MUTANT M35: SDL] DBDevice.deleteOne deleted (device not removed)",
        description="[SDL] deleteDevice: skip DBDevice.deleteOne (device remains in DB)",
        killing_tests=["test10_deleteDevice"],
    ),
    Mutation(
        id="M36", operator="SDL",
        file="controllers/DeviceController.js",
        original="      await DBUserDeviceLinks.deleteMany({ deviceId });",
        mutant="      // [MUTANT M36: SDL] deleteMany UserDeviceLinks deleted",
        description="[SDL] deleteDevice: skip deleteMany user-device-links (links remain)",
        killing_tests=["test10_deleteDevice","test04_getMyDevices"],
    ),
    Mutation(
        id="M37", operator="SDL",
        file="controllers/DeviceController.js",
        original='      await DBSessions.updateMany({ deviceId }, { $set: { terminated: true } });',
        mutant="      // [MUTANT M37: SDL] DBSessions.updateMany deleted (sessions not terminated)",
        description="[SDL] deleteDevice: skip terminating sessions on device delete",
        killing_tests=["test10_deleteDevice","test09_terminateSession"],
    ),
    Mutation(
        id="M38", operator="CDL",
        file="controllers/DeviceController.js",
        original='      return res.status(403).json({ error: "You do not have access to this device" });',
        mutant='      return res.status(200).json({ error: "You do not have access to this device" });',
        description="[CDL] deleteDevice: 403 → 200 on no-access",
        killing_tests=["test10_deleteDevice"],
    ),
    Mutation(
        id="M39", operator="CDL",
        file="controllers/DeviceController.js",
        original='      res.status(200).json({ message: "Device deleted successfully" })',
        mutant='      res.status(201).json({ message: "Device deleted successfully" })',
        description="[CDL] deleteDevice: success 200 → 201",
        killing_tests=["test10_deleteDevice"],
    ),
    Mutation(
        id="M40", operator="CDL",
        file="controllers/DeviceController.js",
        original='      res.status(200).json({ message: "Device link deleted successfully" })',
        mutant='      res.status(201).json({ message: "Device link deleted successfully" })',
        description="[CDL] deleteDevice (user branch): success 200 → 201",
        killing_tests=["test10_deleteDevice"],
    ),

    # ── getMyDevices / getOtherDevices ─────────────────────────────────
    Mutation(
        id="M41", operator="CDL",
        file="controllers/DeviceController.js",
        original='    const myDevices = await DBUserDeviceLinks.find({ userId, role: "owner" })',
        mutant='    const myDevices = await DBUserDeviceLinks.find({ userId, role: "user" })',
        description="[CDL] getMyDevices: 'owner' → 'user' role filter (wrong devices returned)",
        killing_tests=["test04_getMyDevices","test18_devicesAndSession"],
    ),
    Mutation(
        id="M42", operator="CDL",
        file="controllers/DeviceController.js",
        original='    const myDevices = await DBUserDeviceLinks.find({ userId, role: "user" })',
        mutant='    const myDevices = await DBUserDeviceLinks.find({ userId, role: "owner" })',
        description="[CDL] getOtherDevices: 'user' → 'owner' role filter (wrong devices returned)",
        killing_tests=["test11_getOtherDevices"],
    ),

    # ═══════════════════════════════════════════════════════════════════
    # APP CONTROLLER  (controllers/AppController.js)
    # ═══════════════════════════════════════════════════════════════════

    # ── verifyOtp ──────────────────────────────────────────────────────
    Mutation(
        id="M43", operator="ROR",
        file="controllers/AppController.js",
        original="    if (otpData.expiresAt < Date.now()) {",
        mutant="    if (otpData.expiresAt > Date.now()) {",
        description="[ROR] verifyOtp: expiresAt < Date.now() → > (rejects valid OTPs, accepts expired)",
        killing_tests=["test03_registerDevice","test06_createSession","test07_joinSession"],
    ),
    Mutation(
        id="M44", operator="ROR",
        file="controllers/AppController.js",
        original="    if (otpData.expiresAt < Date.now()) {",
        mutant="    if (otpData.expiresAt <= Date.now()) {",
        description="[ROR] verifyOtp: expiresAt < → <= (off-by-one: rejects OTP at exact expiry)",
        killing_tests=["test03_registerDevice"],
    ),
    Mutation(
        id="M45", operator="SDL",
        file="controllers/AppController.js",
        original="""    if (otpData.expiresAt < Date.now()) {
      return res.status(400).json({ message: "OTP expired" });
    }""",
        mutant="    // [MUTANT M45: SDL] OTP expiry check deleted (expired OTPs always valid)",
        description="[SDL] verifyOtp: delete OTP expiry check",
        killing_tests=["test03_registerDevice"],
    ),
    Mutation(
        id="M46", operator="SDL",
        file="controllers/AppController.js",
        original="    await DBOTP.deleteOne({ email, otp });",
        mutant="    // [MUTANT M46: SDL] DBOTP.deleteOne deleted (OTP reusable after first use)",
        description="[SDL] verifyOtp: OTP not deleted after use (replay attack possible)",
        killing_tests=["test03_registerDevice"],
    ),
    Mutation(
        id="M47", operator="CDL",
        file="controllers/AppController.js",
        original='      return res.status(400).json({ message: "Invalid OTP" });',
        mutant='      return res.status(200).json({ message: "Invalid OTP" });',
        description="[CDL] verifyOtp: invalid-OTP 400 → 200",
        killing_tests=["test03_registerDevice"],
    ),
    Mutation(
        id="M48", operator="CDL",
        file="controllers/AppController.js",
        original='      return res.status(400).json({ message: "OTP expired" });',
        mutant='      return res.status(200).json({ message: "OTP expired" });',
        description="[CDL] verifyOtp: expired-OTP 400 → 200",
        killing_tests=["test03_registerDevice"],
    ),
    Mutation(
        id="M49", operator="CDL",
        file="controllers/AppController.js",
        original='      const userDeviceLink = await DBUserDeviceLinks.findOne({ deviceId , role: "owner"});',
        mutant='      const userDeviceLink = await DBUserDeviceLinks.findOne({ deviceId , role: "user"});',
        description="[CDL] verifyOtp: 'owner' → 'user' in existing-link check (wrong role check)",
        killing_tests=["test03_registerDevice"],
    ),
    Mutation(
        id="M50", operator="SDL",
        file="controllers/AppController.js",
        original="""    // Create a new device in the database and a new link for the user
    await DBDevice.create({_id: deviceId, status: "online"});""",
        mutant="    // [MUTANT M50: SDL] DBDevice.create deleted (device not persisted)",
        description="[SDL] verifyOtp: skip DBDevice.create (new device not saved)",
        killing_tests=["test04_getMyDevices","test05_getDeviceInfo","test06_createSession"],
    ),
    Mutation(
        id="M51", operator="SDL",
        file="controllers/AppController.js",
        original="      await DBUserDeviceLinks.create({\n        deviceId,\n        userId: otpData.userId,\n        role: \"owner\"\n      });\n      return res.status(200).json({ message: \"OTP verified\"});\n    } \n    // Create a new device in the database",
        mutant="      // [MUTANT M51: SDL] UserDeviceLinks.create deleted (existing-device path)\n      return res.status(200).json({ message: \"OTP verified\"});\n    } \n    // Create a new device in the database",
        description="[SDL] verifyOtp: skip UserDeviceLinks.create on existing device path",
        killing_tests=["test04_getMyDevices","test05_getDeviceInfo"],
    ),

    # ── sendOtp ────────────────────────────────────────────────────────
    Mutation(
        id="M52", operator="AOR",
        file="controllers/AppController.js",
        original="  await DBOTP.create({ email, otp, expiresAt: Date.now() + 5 * 60 * 1000 , userId});",
        mutant="  await DBOTP.create({ email, otp, expiresAt: Date.now() - 5 * 60 * 1000 , userId});",
        description="[AOR] sendOtp: + → - in expiresAt (OTP always already expired)",
        killing_tests=["test03_registerDevice","test04_getMyDevices","test06_createSession"],
    ),
    Mutation(
        id="M53", operator="AOR",
        file="controllers/AppController.js",
        original="  await DBOTP.create({ email, otp, expiresAt: Date.now() + 5 * 60 * 1000 , userId});",
        mutant="  await DBOTP.create({ email, otp, expiresAt: Date.now() + 50 * 60 * 1000 , userId});",
        description="[AOR] sendOtp: 5 min → 50 min OTP expiry (weakened security)",
        killing_tests=["test03_registerDevice"],
    ),

    # ── logoutApp ──────────────────────────────────────────────────────
    Mutation(
        id="M54", operator="SDL",
        file="controllers/AppController.js",
        original="      await DBUserDeviceLinks.deleteMany({ deviceId });",
        mutant="      // [MUTANT M54: SDL] deleteMany deleted in logoutApp (links remain after logout)",
        description="[SDL] logoutApp: skip deleteMany (device links not removed on logout)",
        killing_tests=["test10_deleteDevice","test04_getMyDevices"],
    ),
]

# ─────────────────────────────────────────────────────────────────────────────
# Core functions
# ─────────────────────────────────────────────────────────────────────────────

def apply_mutation(mutation: Mutation) -> bool:
    """Apply mutation to a copy of the source file. Returns True on success."""
    os.makedirs(mutation.mutant_dir, exist_ok=True)

    with open(mutation.source_path, "r") as f:
        content = f.read()

    if mutation.original not in content:
        print(f"  [WARN] {mutation.id}: original text not found in {mutation.file}")
        return False

    mutated = content.replace(mutation.original, mutation.mutant, 1)
    with open(mutation.mutant_file, "w") as f:
        f.write(mutated)

    # Write a metadata file
    meta_path = os.path.join(mutation.mutant_dir, "meta.txt")
    with open(meta_path, "w") as f:
        f.write(f"Mutation ID  : {mutation.id}\n")
        f.write(f"Operator     : {mutation.operator}\n")
        f.write(f"File         : {mutation.file}\n")
        f.write(f"Description  : {mutation.description}\n")
        f.write(f"Killing tests: {', '.join(mutation.killing_tests)}\n")

    return True


def run_testgen_against_mutant(mutation: Mutation) -> bool:
    """
    Swap in the mutant file, run TestGen, restore original.
    Returns True if mutant is killed (TestGen detects the defect).
    """
    orig_content = open(mutation.source_path).read()

    try:
        # Swap in mutant
        shutil.copy(mutation.mutant_file, mutation.source_path)

        # Run TestGen in FULL_PIPELINE against ghostsocket backend
        result = subprocess.run(
            [TESTGEN_BIN, "ghostsocket"],
            capture_output=True, text=True, timeout=120
        )
        output = result.stdout + result.stderr

        # A mutant is killed if ANY test reports FAILED or gets an unexpected response
        killed = ("FAILED" in output or "✗" in output or
                  "assertion" in output.lower() or result.returncode != 0)
        return killed

    except subprocess.TimeoutExpired:
        print(f"  [TIMEOUT] {mutation.id}")
        return False
    finally:
        # Always restore original
        with open(mutation.source_path, "w") as f:
            f.write(orig_content)


def print_report(mutations, run_results=None):
    """Print a formatted mutation testing report."""

    total = len(mutations)
    if run_results:
        killed_count = sum(1 for m in mutations if m.killed)
        score = killed_count / total * 100 if total else 0

    print("=" * 72)
    print("  TESTGEN MUTATION TESTING REPORT")
    print("  Subject Under Test : GhostSocket Node.js Backend")
    print(f"  Total Mutants      : {total}")
    if run_results:
        print(f"  Killed             : {killed_count}")
        print(f"  Survived           : {total - killed_count}")
        print(f"  Mutation Score     : {score:.1f}%")
    print("=" * 72)

    by_operator = {}
    for m in mutations:
        by_operator.setdefault(m.operator, []).append(m)

    for op, muts in sorted(by_operator.items()):
        print(f"\n── {op} Mutations ({len(muts)}) ──")
        for m in muts:
            status = ""
            if m.killed is True:
                status = "  KILLED   ✓"
            elif m.killed is False:
                status = "  SURVIVED ✗"
            print(f"  {m.id}  {m.description[:62]:<62}{status}")

    if run_results:
        print("\n" + "=" * 72)
        print(f"  FINAL MUTATION SCORE: {killed_count}/{total} = {score:.1f}%")
        print("=" * 72)


# ─────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="TestGen Mutation Testing")
    parser.add_argument("--apply",       action="store_true", help="Create mutant files")
    parser.add_argument("--report",      action="store_true", help="Print catalogue report")
    parser.add_argument("--run-testgen", action="store_true", help="Run TestGen against each mutant")
    args = parser.parse_args()

    if args.apply:
        print(f"Creating {len(MUTATIONS)} mutant files in {MUTANTS_DIR}/")
        ok = failed = 0
        for m in MUTATIONS:
            if apply_mutation(m):
                print(f"  ✓ {m.id}  ({m.operator})  {os.path.basename(m.file)}")
                ok += 1
            else:
                print(f"  ✗ {m.id}  FAILED")
                failed += 1
        print(f"\nApplied: {ok}  Failed: {failed}")

    if args.run_testgen:
        print(f"\nRunning TestGen against {len(MUTATIONS)} mutants…")
        for m in MUTATIONS:
            if not os.path.exists(m.mutant_file):
                print(f"  [SKIP] {m.id} — mutant file not found, run --apply first")
                continue
            killed = run_testgen_against_mutant(m)
            m.killed = killed
            status = "KILLED" if killed else "SURVIVED"
            print(f"  {m.id}  {status}  {m.description[:55]}")

    if args.report:
        print_report(MUTATIONS, run_results=args.run_testgen)

    if not any([args.apply, args.report, args.run_testgen]):
        print(f"Loaded {len(MUTATIONS)} mutations.  Use --apply, --report, or --run-testgen.")
        print_report(MUTATIONS)


if __name__ == "__main__":
    main()
