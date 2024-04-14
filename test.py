import os
import time
import psutil
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

# Set up InfluxDB connection details
token = os.environ.get("INFLUXDB_TOKEN")
org = "root"
url = "http://localhost:8086"

# Initialize InfluxDB client
write_client = InfluxDBClient(url=url, token=token, org=org)

# Set the bucket
bucket = "root"

# Initialize write API
write_api = write_client.write_api(write_options=SYNCHRONOUS)

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

# Write CPU and memory utilization to InfluxDB
while True:
    cpu_percent, memory_percent = get_system_stats()
    
    # Create a data point
    point = (
        Point("system_stats")
        .tag("host", "localhost")  # Tag to identify the host
        .field("cpu_percent", cpu_percent)
        .field("memory_percent", memory_percent)
        .field("client_count", get_number_of_clients())
        .field("server_active_clients", get_active_clients('server.txt'))
        .field("mirror1_active_clients", get_active_clients('mirror1.txt'))
        .field("mirror2_active_clients", get_active_clients('mirror2.txt'))
    )
    print(get_active_clients('server.txt'))
    
    # Write the data point to InfluxDB
    write_api.write(bucket=bucket, org=org, record=point)
    
    # Wait for 1 second before next measurement
    time.sleep(1)
