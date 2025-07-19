# Multi-stage build for V8 C++ Integration
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    libv8-dev \
    libgtest-dev \
    git \
    python3 \
    python3-pip \
    pkg-config \
    libssl-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the project
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_V8=ON -G Ninja
RUN cmake --build build --parallel

# Production stage
FROM ubuntu:22.04 AS production

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libv8-dev \
    libstdc++6 \
    libc6 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN groupadd -r v8user && useradd -r -g v8user v8user

# Set working directory
WORKDIR /app

# Copy built executables and scripts
COPY --from=builder /app/build/system_v8_example /app/
COPY --from=builder /app/build/v8_example /app/
COPY --from=builder /app/build/advanced_example /app/
COPY --from=builder /app/build/test_suite /app/
COPY --from=builder /app/build/advanced_test_suite /app/
COPY --from=builder /app/ShellScripts/run_tests.sh /app/

# Copy configuration files
COPY --from=builder /app/config/ /app/config/
COPY --from=builder /app/docs/ /app/docs/

# Set permissions
RUN chmod +x /app/ShellScripts/run_tests.sh && \
    chmod +x /app/*_example && \
    chmod +x /app/test_suite && \
    chmod +x /app/advanced_test_suite && \
    chown -R v8user:v8user /app

# Switch to non-root user
USER v8user

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD /app/system_v8_example || exit 1

# Default command
CMD ["./ShellScripts/run_tests.sh"]

# Development stage
FROM builder AS development

# Install development tools
RUN apt-get update && apt-get install -y \
    gdb \
    valgrind \
    clang-tidy \
    cppcheck \
    doxygen \
    graphviz \
    strace \
    && rm -rf /var/lib/apt/lists/*

# Install additional development dependencies
RUN pip3 install \
    conan \
    pre-commit \
    black \
    flake8

# Set development environment
ENV NODE_ENV=development
ENV V8_DEBUG=1

# Create development user
RUN groupadd -r devuser && useradd -r -g devuser -s /bin/bash devuser
RUN usermod -aG sudo devuser

WORKDIR /app

# Copy all source files for development
COPY --from=builder /app /app

# Set permissions for development
RUN chown -R devuser:devuser /app

USER devuser

CMD ["/bin/bash"]