import os
from kafka import KafkaConsumer
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

# Set up Kafka connection details
bootstrap_servers = 'localhost:9092'
topic = 'system_stats'

# Initialize Kafka consumer
consumer = KafkaConsumer(topic,
                         bootstrap_servers=bootstrap_servers,
                         group_id='system_stats_group')

# Define a function to write data points to InfluxDB
def write_to_influxdb(message):
    point = (
        Point("system_stats")
        .tag("host", "localhost")  # Tag to identify the host
        .field("cpu_percent", message.get("cpu_percent"))
        .field("memory_percent", message.get("memory_percent"))
        .field("client_count", message.get("client_count"))
        .field("server_active_clients", message.get("server_active_clients"))
        .field("mirror1_active_clients", message.get("mirror1_active_clients"))
        .field("mirror2_active_clients", message.get("mirror2_active_clients"))
    )
    write_api.write(bucket=bucket, org=org, record=point)

# Consume messages from Kafka and write them to InfluxDB
for message in consumer:
    # Decode the message value from bytes to dictionary
    message_value = eval(message.value.decode('utf-8'))
    print(message_value)
    
    # Write the message data to InfluxDB
    write_to_influxdb(message_value)
