import java.util.*;

// Represents a node in the Huffman tree
class HuffmanNode {
    int frequency; // Frequency of the character
    char character; // The character represented by the node
    HuffmanNode left; // Left child in the Huffman tree
    HuffmanNode right; // Right child in the Huffman tree
}

// Class for encoding and decoding using Huffman coding
public class HuffmanEncoding {
    private Map<Character, String> huffmanCodeMap = new HashMap<>(); // Map to store character to code mapping
    private Map<String, Character> reverseHuffmanCodeMap = new HashMap<>(); // Map to store code to character mapping
    
    // Builds a Huffman tree based on character frequencies in the given text
    public void buildTree(String text) {
        // Calculate frequency of each character in the text
        Map<Character, Integer> frequencyMap = new HashMap<>();
        for (char c : text.toCharArray()) {
            frequencyMap.put(c, frequencyMap.getOrDefault(c, 0) + 1);
        }

        // Priority queue to construct the Huffman tree
        PriorityQueue<HuffmanNode> pq = new PriorityQueue<>(Comparator.comparingInt(node -> node.frequency));
        for (Map.Entry<Character, Integer> entry : frequencyMap.entrySet()) {
            HuffmanNode node = new HuffmanNode();
            node.character = entry.getKey();
            node.frequency = entry.getValue();
            pq.add(node);
        }

        // Combine nodes to form the Huffman tree
        while (pq.size() > 1) {
            HuffmanNode left = pq.poll();
            HuffmanNode right = pq.poll();
            HuffmanNode newNode = new HuffmanNode();
            newNode.frequency = left.frequency + right.frequency;
            newNode.left = left;
            newNode.right = right;
            pq.add(newNode);
        }

        // Generate Huffman codes for each character
        generateCodes(pq.poll(), "");
    }

    // Recursively generates codes for characters in the Huffman tree
    private void generateCodes(HuffmanNode node, String code) {
        if (node == null) return;

        if (node.left == null && node.right == null) {
            huffmanCodeMap.put(node.character, code); // Map character to code
            reverseHuffmanCodeMap.put(code, node.character); // Map code to character
        }

        generateCodes(node.left, code + "0"); // Traverse left subtree
        generateCodes(node.right, code + "1"); // Traverse right subtree
    }

    // Encodes the given text using the Huffman codes
    public String encode(String text) {
        StringBuilder encoded = new StringBuilder();
        for (char c : text.toCharArray()) {
            encoded.append(huffmanCodeMap.get(c)); // Replace each character with its code
        }
        return encoded.toString();
    }

    // Decodes the given encoded text back into the original text
    public String decode(String encodedText) {
        StringBuilder decoded = new StringBuilder();
        String temp = "";
        for (char c : encodedText.toCharArray()) {
            temp += c;
            if (reverseHuffmanCodeMap.containsKey(temp)) { // Check if code is complete
                decoded.append(reverseHuffmanCodeMap.get(temp)); // Get the character for the code
                temp = ""; // Reset temp for next code
            }
        }
        return decoded.toString();
    }
}
