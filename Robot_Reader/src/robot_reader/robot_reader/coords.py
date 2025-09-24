from math import sin, cos, pi, atan2

def rel2abs(x, y, z, qz, qw, origin_x, origin_y, origin_z, rotation=0.0):
    """!
    Returns x, y, z from a system with origin in coordinates
    (origin_x, origin_y, origin_z) rotated by qz, qw in absolute coordinates.
    """
    max_rotation = 2 * pi
    siny_cosp = 2 * (qw * qz)
    cosy_cosp = 1 - 2 * (qz * qz)
    angle = atan2(siny_cosp, cosy_cosp)
    angle = (angle + rotation) % max_rotation
    new_x = x*cos(rotation)-y*sin(rotation)+(origin_x)
    new_y = x*sin(rotation)+y*cos(rotation)+(origin_y)
    new_z = z + origin_z  # Aggiunta la coordinata z
    return round(new_x, 2), round(new_y, 2), round(new_z, 2), round(angle, 2)