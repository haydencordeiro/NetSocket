import os
import time
import psutil
import json  # Import the json module to encode the message as JSON
from kafka import KafkaProducer

# Set up Kafka connection details
bootstrap_servers = 'localhost:9092'
topic = 'system_stats'

# Initialize Kafka producer
producer = KafkaProducer(bootstrap_servers=bootstrap_servers)

# Define a function to get CPU and memory utilization
def get_system_stats():
    cpu_percent = psutil.cpu_percent()
    memory_percent = psutil.virtual_memory().percent
    return cpu_percent, memory_percent

def get_number_of_clients():
    try:
        with open('count.txt', 'r') as file:
            # Read the integer value from the file
            count_value = int(file.read())
            return count_value
    except:
        pass
    return 0

def get_active_clients(filename):
    try:
        with open(filename, 'r') as file:
            # Read the integer value from the file
            count_value = int(file.read())
            return count_value
    except:
        pass
    return 0

# Write CPU and memory utilization to Kafka topic
while True:
    cpu_percent, memory_percent = get_system_stats()
    
    # Construct the message payload
    message = {
        "cpu_percent": cpu_percent,
        "memory_percent": memory_percent,
        "client_count": get_number_of_clients(),
        "server_active_clients": get_active_clients('server.txt'),
        "mirror1_active_clients": get_active_clients('mirror1.txt'),
        "mirror2_active_clients": get_active_clients('mirror2.txt')
    }
    
    # Encode the message as JSON and then as bytes
    message_bytes = json.dumps(message).encode('utf-8')
    
    # Publish the message to Kafka topic
    producer.send(topic, value=message_bytes)
    
    # Wait for 1 second before next measurement
    time.sleep(1)
