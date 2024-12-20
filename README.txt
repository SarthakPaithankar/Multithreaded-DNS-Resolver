sem_initiator(){
    sem_initiator all semaphores and mutexes needed for the program to function
}

get_filename(){
    get file mutex locks the file names array and safely pull out one file for the threads to access at a time
}

Record parse(){
    Record parse mutex locks the requestor file and writes to the buffer so no race conditions are met
}

conversion log(){
    conversion log safely takes names out of the buffer resolves them and writes them to the resolved file
}