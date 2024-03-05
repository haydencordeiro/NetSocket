#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096*10



// Function to create a new PDF file from an existing PDF file
void createNewPDF(const char *inputFilename, const char *outputFilename) {
    int input_fd, output_fd;
    ssize_t nread;
    char buffer[BUFFER_SIZE];

    // Open the input PDF file for reading
    input_fd = open(inputFilename, O_RDONLY);
    if (input_fd == -1) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    // Create the output PDF file for writing
    output_fd = open(outputFilename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_fd == -1) {
        perror("Error creating output file");
        exit(EXIT_FAILURE);
    }

    // Read from the input file and write to the output file
    while ((nread = read(input_fd, buffer, BUFFER_SIZE)) > 0) {
        printf("Reading\n");
        if (write(output_fd, buffer, nread) != nread) {
            perror("Error writing to output file");
            exit(EXIT_FAILURE);
        }
    }

    // Close the files
    if (close(input_fd) == -1) {
        perror("Error closing input file");
        exit(EXIT_FAILURE);
    }
    if (close(output_fd) == -1) {
        perror("Error closing output file");
        exit(EXIT_FAILURE);
    }
}

int main() {
    const char *inputFile = "./serverDir/temp.pdf";
    const char *outputFile = "output.pdf";

 

    // Create a new PDF file based on the input PDF file
    createNewPDF(inputFile, outputFile);
    printf("New PDF file created: %s\n", outputFile);

    return 0;
}
