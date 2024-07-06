# Network File System
For detailed documentation visit [NFS Documentation](https://santhosh.gitbook.io/nfs).

## Overview
- This project aims to develop a network file system that enables seamless interaction between clients and storage servers.   
- The NFS architecture consists of three major components: Clients, Naming Server, and Storage Servers. For a detailed overview of the components, visit [NFS Components](https://santhosh.gitbook.io/nfs/overview/components).

## Installation
- Clone the repo
  ```bash
  git clone https://github.com/Santhosh-2205/NFS
- Navigate to cloned repository
  ```bash
  cd NFS
- Compile files
  ```bash
  make all

## Running

### Naming Server
Only one naming server exists which acts as the center point between all Storage servers and Clients.
- **To run**: `./nm`

### Storage Servers
Multiple storage servers can be spawned from any directory.
- **To run**: `./ss`
- Once running, you will be prompted to enter 3 port numbers (for communication with NM, Client, and other Storage servers). Enter 3 unoccupied ports.
- Then you will be prompted to enter `ss_id`:
    - If starting a new server, enter `-1`. You will receive a unique `ss_id` for the storage server.
    - If retrieving an old (lost/killed) storage server, enter the `ss_id` of the storage server you want to retrieve. It will then retrieve contents from the redundant storage server if it exists.
    - Enter `-2` or `-3` if you want this server to be a redundant storage server. A maximum of 2 redundant storage servers can exist. Directories named `RS0` and `RS1` will be created.
- A directory named `SS<ss_id>` will be created in the directory from where the storage server is spawned. This directory will store all the created files and directories.

### Client
Multiple clients can be spawned.
- **To run**: `./client`
- Once running, you will be displayed instructions.
- For detailed usage visit [NFS Client Usage](https://santhosh.gitbook.io/nfs/product-guides/usage)



