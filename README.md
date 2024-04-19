
# NetSocket
![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Sockets](https://img.shields.io/badge/Sockets-007396?style=for-the-badge&logo=socket.io&logoColor=white)
![Graphana](https://img.shields.io/badge/Graphana-F46800?style=for-the-badge&logo=grafana&logoColor=white)
![Influx DB](https://img.shields.io/badge/Influx%20DB-22ADF6?style=for-the-badge&logo=influxdb&logoColor=white)
![Redpanda](https://img.shields.io/badge/Redpanda-000000?style=for-the-badge&logo=apachekafka&logoColor=white)
![Kafka](https://img.shields.io/badge/Kafka-231F20?style=for-the-badge&logo=apachekafka&logoColor=white)
![Python](https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white)


## Overview

#### NetSocket - Client Server Architecture with Load Balancing and File Transfer using Sockets

This project presents a robust client-server architecture implemented in C, facilitating multiple clients to interact with a server system efficiently. The server, termed as serverw24 along with its mirror instances, enables clients to execute various commands, including file retrieval, searching, and downloading, via socket connections. Load balancing is achieved through a Round Robin mechanism, ensuring equitable distribution of client requests among server instances.

The system architecture also incorporates real-time monitoring and visualization capabilities. Statistics such as the number of connected clients, CPU and memory utilization are collected and pushed into a Kafka queue, then stored in InfluxDB for persistent storage. Grafana is employed for visualization and monitoring purposes, offering stakeholders valuable insights into system performance and resource utilization.

The project adheres to strict guidelines, including individual or team-based project development, plagiarism checks via MOSS, and scheduled demonstrations followed by viva sessions. Submission includes well-commented C files for server, client, and mirror instances, ensuring clarity and ease of understanding for future development and maintenance.

Overall, this project offers a comprehensive solution for scalable client-server communication, incorporating load balancing and real-time monitoring functionalities to meet the demands of modern networked environments.

## Screenshots
<img src="./screenshots/1.jpg" width="60%" /> <br>

## Demo
[![DEMO](https://img.youtube.com/vi/R24t-8zrYQU/0.jpg)](https://www.youtube.com/watch?v=R24t-8zrYQU)

## Contributions <a id="contributions"></a>
![Alt](https://repobeats.axiom.co/api/embed/8f067b3de758710566b9d73f68f1778424ce633d.svg "Repobeats analytics image")

## Contributors <a id="contributors"></a>
  - [Hayden Cordeiro](https://hayden.co.in/)<br>
  [![Linkedin](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/haydencordeiro/)
  [![Github](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/haydencordeiro)

- [Jivin Varghese Porthukaran](https://jivin.co.in/)<br>
  [![Linkedin](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/JivinVarghese/)
  [![Github](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/JivinVarghese)
