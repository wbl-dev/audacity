function resizeEdgeAt(x, left, right) {
    const width = Math.max(0, right - left)
    const innerEdge = Math.min(3, width * 0.2)
    if (x >= left - 8 && x <= left + innerEdge) {
        return -1
    }
    if (x >= right - innerEdge && x <= right + 8) {
        return 1
    }
    return 0
}
