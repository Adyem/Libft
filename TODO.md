# TODO

- Math / Linear Algebra Module
    - Vectors (2D, 3D, 4D) with dot, cross, normalize, length
    - Matrices (2x2, 3x3, 4x4) for multiply, invert, transform
    - Quaternions for 3D rotation handling
    - Utility functions: min, max, clamp, lerp, smoothstep

- Time / Date Module
    - Current time in ms, ns, epoch
    - Time formatting and parsing
    - Timers, countdowns, and sleep wrappers

- Compression Module
    - ft_compress and ft_decompress
    - Stream compression
    - Base64 encode and decode

- Image / Media Module
    - Load and save PNG or BMP
    - Image buffers storing width, height, RGBA
    - Drawing primitives: blit, fill, resize, grayscale
    - Audio buffer loading

- Math Expression / Scripting Module
    - Replace variables (x = 10; "x * 2" = 20)
    - Extend into config scripting

- Concurrency / Task Module
    - Thread pool and futures
    - Job scheduling for delayed or recurring tasks
    - Lock-free queues for producer and consumer

- Database / Persistent Storage Module
    - Lightweight key-value store
    - JSON-backed storage with CRUD helpers
    - Cache system with load and save

- Geometry / Collision Module
    - AABB, circle, sphere, OBB shapes
    - Collision detection and intersection math

- Unit Testing / Assert Module
    - ft_assert integrated with logger
    - Simple test runner framework
    - Auto-log failures with file and line
