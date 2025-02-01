# Use an official lightweight C++ image
FROM ubuntu:latest

# Set environment variables to avoid user prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Update system and install dependencies
RUN apt update && apt upgrade -y && \
    apt install -y g++ cmake make libboost-all-dev libssl-dev libasio-dev wget curl \
    && rm -rf /var/lib/apt/lists/*

# Set working directory inside the container
WORKDIR /app

# Copy all source files to the container
COPY . /app

# Compile the Crow application
RUN g++ -I dependencies -I utils main.cpp utils/FileUtils.cpp utils/JsonUtils.cpp -o server -lpthread -std=c++17

# Expose the Crow server port
EXPOSE 8080

# Run the Crow server
CMD ["./server"]
