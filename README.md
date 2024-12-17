# Node Editor
![editor4](https://github.com/user-attachments/assets/f8795e75-5788-4e2d-ac25-2f72ec651e5e)



## Overview

Node Editor is a graphical application inspired by **ImNodes** that allows users to create and interact with nodes in a visual programming environment. 
Nodes represent operations, values, or time-based computations, and can be connected to simulate complex behaviors. 
The project is designed for flexibility and real-time interaction, supporting various node types like arithmetic operations, 
color outputs, and a central **Viewport Node** for visualizing connected node outputs.

---

## Features

- **Visual Node Editor:**
  - Drag-and-drop interface for creating and connecting nodes.
  - Supports adding, deleting, and editing nodes.

- **Supported Node Types:**
  - **Add Node**: Computes the sum of two inputs.
  - **Multiply Node**: Computes the product of two inputs.
  - **Sine Node**: Computes the sine of an input value.
  - **Time Node**: Outputs the current time.
  - **Output Node**: Combines RGB values to display a color preview.
  - **Viewport Node**: Acts as a centralized screen for visualizing the state of connected nodes.

- **Dynamic Value Updates:**
  - Node values and connections are updated in real-time.

- **Customizable UI:**
  - Switch between Classic, Dark, and Light themes.
  - Includes a minimap for quick navigation.

---

## Getting Started

### Prerequisites

- A C++ compiler with C++17 support.
- Libraries:
  - **ImGui**: GUI rendering.
  - **ImNodes**: Node-based graphical interface.
- A build system like **CMake**.

### Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/Martinfx/materialeditor.git
   cd materialeditor

### Usage Instructions

1. Adding Nodes:
 - Right-click in the editor and select a node type from the menu.
 - Supported node types include Add, Multiply, Sine, Time, Output, and Viewport.

2. Connecting Nodes:
 - Drag a link from one node's output to another node's input.

3. Viewport Node:
 - Connect any node to the Viewport Node to visualize its output in a dedicated screen.

4. Styling and Minimap:
 - Use the menu bar to switch styles or position the minimap.

### Inspiration
This project is inspired by the ImNodes library and its elegant API for creating node-based editors. 
The design incorporates concepts from real-time computation graphs, 
making it suitable for visual programming applications like shaders, animations, and procedural generation.

### Acknowledgements

## Special thanks to:

 - **ImNodes: For providing the core node-based UI inspiration.
 - **ImGui: For the flexible and lightweight GUI framework.
