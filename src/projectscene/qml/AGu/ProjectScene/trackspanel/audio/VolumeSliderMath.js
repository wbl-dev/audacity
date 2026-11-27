function clampVolume(value, from, to) {
    return Math.max(from, Math.min(to, value))
}

function snapVolume(value, shiftPressed, snapRange) {
    return !shiftPressed && Math.abs(value) < (snapRange === undefined ? 1.2 : snapRange) ? 0 : value
}

function volumeAtPointer(pointerX, width, handleWidth, handleOffset, from, to) {
    const travel = Math.max(1, width - handleWidth)
    const relative = (pointerX - handleOffset - handleWidth / 2) / travel
    return clampVolume(from + relative * (to - from), from, to)
}
