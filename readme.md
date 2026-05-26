# Vector Database Engine

A high-performance in-memory vector database built in C++ that supports dense vector ingestion, brute-force nearest-neighbor search, and accelerated Inverted File Index (IVF) searches using K-Means clustering.

## How to Build the Project

Compile the server, client, and benchmark binaries separately using `make` with a different payload for each:

### Compile Server

Bash

```
make sb #Compile
make sr #Run
```

### Compile Client

Bash

```
make cb #Compile
make cr #Run
```

### Compile Benchmark

Bash

```
make bb #Compile
make br #Run
```


## Supported Commands and Format

Communication uses a plain-text, line-based protocol over TCP. The following commands can be typed directly into the client prompt:

-   **`ADD <id> <v_1> <v_2> ... <v_D>`**
    
    -   _Description:_ Inserts or overwrites a single vector with a unique 64-bit integer ID.
        
    -   _Example:_ `ADD 1 0.10 0.20 0.30 0.40`
        
-   **`SEARCH <v_1> <v_2> ... <v_D> <k> <mode> [nprobe]`**
    
    -   _Description:_ Returns the top-$k$ closest vectors using squared Euclidean distance (or Cosine distance if flagged).
        
    -   _Modes:_ `BRUTE` (scans all vectors) or `IVF` (scans nearest cluster partitions). `nprobe` defines how many clusters to search in `IVF` mode.
        
    -   _Example:_ `SEARCH 0.12 0.22 0.32 0.42 3 IVF 1`
        
-   **`BUILD`**
    
    -   _Description:_ Triggers the K-Means clustering algorithm to construct or refresh the IVF index. The number of clusters is dynamically calculated as $\lfloor\sqrt{N}\rfloor$.
        
-   **`SAVE`**
    
    -   _Description:_ Persists the core vector store and current IVF index to a binary snapshot file (`snapshot.vdb`) using a safe temporary-write-and-rename pattern.
        
-   **`LOAD`**
    
    -   _Description:_ Wipes current memory and loads the vector store and index back from the binary snapshot file.
        
-   **`STATS`**
    
    -   _Description:_ Outputs configuration details, including vector dimensionality, total element count, index status, and individual cluster group sizes.
        
-   **`QUIT`**
    
    -   _Description:_ Safely disconnects the client session while leaving the background database server running.
        

## Known System Limitations

-   **Input Filtering Constraints:** The command-line client parsing loop performs minimal text sanitization. Client prompts are not completely filtered, meaning that malformed text sequences, extra whitespace handling edge cases, or bad string inputs can cause unexpected exceptions or unexpected client disconnections.
    
-   **Static Real-Time Boundaries:** Ingesting vectors after running a `BUILD` command routes the incoming data into the single nearest static cluster centroid. Over time, heavy updates will degrade search accuracy until a fresh `BUILD` command is called to rebalance the cluster partitions.
