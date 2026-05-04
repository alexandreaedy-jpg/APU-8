autowatch = 1;
outlets = 14;

var targetIndex = 0;

var TARGETS = [
    {
        name: "P1",
        channel: 12,
        ctrls: [1, 76, 5, 16],
        ctrlLabels: ["Vib Depth", "Vib Rate", "Glide", "Duty (0-3)"],
        state: { attack: 0, decay: 32, sustain: 96, release: 24, ctrl1: 0, ctrl2: 0, ctrl3: 0, ctrl4: 2 }
    },
    {
        name: "P2",
        channel: 13,
        ctrls: [1, 76, 5, 16],
        ctrlLabels: ["Vib Depth", "Vib Rate", "Glide", "Duty (0-3)"],
        state: { attack: 0, decay: 32, sustain: 96, release: 24, ctrl1: 0, ctrl2: 0, ctrl3: 0, ctrl4: 2 }
    },
    {
        name: "TRI",
        channel: 14,
        ctrls: [1, 76, 5, 28],
        ctrlLabels: ["Vib Depth", "Vib Rate", "Glide", "Punch"],
        state: { attack: 0, decay: 32, sustain: 96, release: 24, ctrl1: 0, ctrl2: 0, ctrl3: 0, ctrl4: 127 }
    },
    {
        name: "NOISE",
        channel: 15,
        ctrls: [16, 1, -1, -1],
        ctrlLabels: ["Timbre", "Mode", "-", "-"],
        state: { attack: 0, decay: 32, sustain: 96, release: 24, ctrl1: 0, ctrl2: 0, ctrl3: 0, ctrl4: 0 }
    },
    {
        name: "GLOBAL",
        channel: 16,
        ctrls: [24, 26, 27, 16],
        ctrlLabels: ["Arp On", "Arp Mode", "Arp Speed", "Duty (0-3)"],
        state: { attack: 0, decay: 32, sustain: 96, release: 24, ctrl1: 0, ctrl2: 0, ctrl3: 32, ctrl4: 2 }
    }
];

function clamp(v, lo, hi) {
    v = parseInt(v, 10);
    if (isNaN(v)) {
        v = 0;
    }
    if (v < lo) {
        return lo;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

function currentTarget() {
    return TARGETS[targetIndex];
}

function currentState() {
    return currentTarget().state;
}

function sendCC(channel, cc, value) {
    var status = 0xB0 | ((channel - 1) & 0x0F);
    outlet(0, [status, clamp(cc, 0, 127), clamp(value, 0, 127)]);
}

function sendMain(cc, value) {
    sendCC(currentTarget().channel, cc, value);
}

function sendMappedControl(index, value) {
    var cc = currentTarget().ctrls[index];
    if (cc >= 0) {
        if (cc === 16) {
            value = dutyUiToCc(value);
        }
        sendCC(currentTarget().channel, cc, value);
    }
}

function dutyUiClamp(v) {
    v = clamp(v, 0, 127);
    if (v <= 3) {
        return v;
    }
    if (v < 32) {
        return 0;
    }
    if (v < 64) {
        return 1;
    }
    if (v < 96) {
        return 2;
    }
    return 3;
}

function dutyUiToCc(v) {
    v = dutyUiClamp(v);
    if (v <= 0) {
        return 0;
    }
    if (v === 1) {
        return 43;
    }
    if (v === 2) {
        return 86;
    }
    return 127;
}

function updateUI() {
    var target = currentTarget();
    var state = currentState();

    outlet(1, ["set", target.name + "  (CH" + target.channel + ")"]);
    outlet(2, ["set", target.ctrlLabels[0]]);
    outlet(3, ["set", target.ctrlLabels[1]]);
    outlet(4, ["set", target.ctrlLabels[2]]);
    outlet(5, ["set", target.ctrlLabels[3]]);

    outlet(6, state.attack);
    outlet(7, state.decay);
    outlet(8, state.sustain);
    outlet(9, state.release);
    outlet(10, state.ctrl1);
    outlet(11, state.ctrl2);
    outlet(12, state.ctrl3);
    outlet(13, state.ctrl4);
}

function refresh() {
    var state = currentState();
    sendMain(73, state.attack);
    sendMain(75, state.decay);
    sendMain(71, state.sustain);
    sendMain(72, state.release);
    sendMappedControl(0, state.ctrl1);
    sendMappedControl(1, state.ctrl2);
    sendMappedControl(2, state.ctrl3);
    sendMappedControl(3, state.ctrl4);
}

function selectTarget(index) {
    targetIndex = clamp(index, 0, TARGETS.length - 1);
    updateUI();
    refresh();
}

function target(v) {
    selectTarget(v);
}

function attack(v) {
    currentState().attack = clamp(v, 0, 127);
    sendMain(73, currentState().attack);
}

function decay(v) {
    currentState().decay = clamp(v, 0, 127);
    sendMain(75, currentState().decay);
}

function sustain(v) {
    currentState().sustain = clamp(v, 0, 127);
    sendMain(71, currentState().sustain);
}

function release(v) {
    currentState().release = clamp(v, 0, 127);
    sendMain(72, currentState().release);
}

function ctrl1(v) {
    currentState().ctrl1 = clamp(v, 0, 127);
    sendMappedControl(0, currentState().ctrl1);
}

function ctrl2(v) {
    currentState().ctrl2 = clamp(v, 0, 127);
    sendMappedControl(1, currentState().ctrl2);
}

function ctrl3(v) {
    currentState().ctrl3 = clamp(v, 0, 127);
    sendMappedControl(2, currentState().ctrl3);
}

function ctrl4(v) {
    if (currentTarget().ctrls[3] === 16) {
        currentState().ctrl4 = dutyUiClamp(v);
    } else {
        currentState().ctrl4 = clamp(v, 0, 127);
    }
    sendMappedControl(3, currentState().ctrl4);
}

function panic() {
    sendCC(currentTarget().channel, 120, 0);
    sendCC(currentTarget().channel, 123, 0);
}

function panic_all() {
    var i;
    for (i = 0; i < TARGETS.length; i++) {
        sendCC(TARGETS[i].channel, 120, 0);
        sendCC(TARGETS[i].channel, 123, 0);
    }
}

function anything() {
    var name = messagename.toUpperCase();

    if (name === "P1") {
        selectTarget(0);
        return;
    }
    if (name === "P2") {
        selectTarget(1);
        return;
    }
    if (name === "TRI") {
        selectTarget(2);
        return;
    }
    if (name === "NOISE") {
        selectTarget(3);
        return;
    }
    if (name === "GLOBAL") {
        selectTarget(4);
    }
}

function loadbang() {
    selectTarget(0);
}
