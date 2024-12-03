import javax.swing.*;
import java.awt.*;
import java.io.*;
import java.net.*;

public class HuffmanServer {
    private static JTextArea logArea; // Text area to display logs and messages
    private static JTextField messageField; // Text field to input messages
    private static PrintWriter out; // Output stream for sending messages to the client
    private static HuffmanEncoding huffman; // Huffman encoding instance for encoding and decoding

    public static void main(String[] args) {
        // Setup GUI for the server
        JFrame frame = new JFrame("Huffman Chat Server");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(500, 600);
        frame.setLayout(new BorderLayout(5, 5));

        // Configure the message log area
        logArea = new JTextArea();
        logArea.setEditable(false);
        logArea.setLineWrap(true);
        logArea.setWrapStyleWord(true);
        logArea.setFont(new Font("맑은 고딕", Font.PLAIN, 12));
        logArea.setMargin(new Insets(5, 5, 5, 5));
        logArea.setBackground(new Color(248, 248, 248));
        
        JScrollPane scrollPane = new JScrollPane(logArea);
        scrollPane.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        frame.add(scrollPane, BorderLayout.CENTER);

        // Configure input panel for sending messages
        JPanel inputPanel = new JPanel(new BorderLayout(5, 0));
        inputPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        inputPanel.setPreferredSize(new Dimension(frame.getWidth(), 50));
    
        messageField = new JTextField();
        messageField.setFont(new Font("맑은 고딕", Font.PLAIN, 12));
        messageField.setPreferredSize(new Dimension(messageField.getPreferredSize().width, 40));
        messageField.setEnabled(true);
        messageField.requestFocus();
    
        JButton sendButton = new JButton("전송"); // Send button
        sendButton.setFont(new Font("맑은 고딕", Font.PLAIN, 12));
        sendButton.setPreferredSize(new Dimension(80, 40));
        inputPanel.add(messageField, BorderLayout.CENTER);
        inputPanel.add(sendButton, BorderLayout.EAST);
        frame.add(inputPanel, BorderLayout.SOUTH);

        frame.setVisible(true);
        SwingUtilities.invokeLater(() -> messageField.requestFocus());

        try (ServerSocket serverSocket = new ServerSocket(12345)) {
            appendMessage("시스템", "서버가 포트 12345에서 시작되었습니다...", null);
            Socket socket = serverSocket.accept(); // Wait for client connection
            appendMessage("시스템", "클라이언트가 연결되었습니다.", null);

            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream())); // Input stream from client
            out = new PrintWriter(socket.getOutputStream(), true); // Output stream to client
            huffman = new HuffmanEncoding();
            huffman.buildTree("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890,.!?'~@#$%^&*()_+{}:<>`-=[]|\\;\"/");

            // Thread to handle incoming messages from the client
            Thread receiverThread = new Thread(() -> {
                try {
                    String encodedMessage;
                    while ((encodedMessage = in.readLine()) != null) {
                        String decodedMessage = huffman.decode(encodedMessage); // Decode received message
                        handleReceivedMessage(encodedMessage);

                        if (decodedMessage.equalsIgnoreCase("exit")) {
                            appendMessage("시스템", "클라이언트가 종료되었습니다.", null);
                            System.exit(0); // Exit if client disconnects
                        }
                    }
                } catch (IOException e) {
                    appendMessage("시스템", "수신 오류: " + e.getMessage(), null);
                }
            });

            sendButton.addActionListener(e -> sendMessage()); // Event for send button
            messageField.addActionListener(e -> sendMessage()); // Event for pressing enter in message field
            receiverThread.start(); // Start the receiver thread
            receiverThread.join();

        } catch (IOException | InterruptedException e) {
            appendMessage("시스템", "오류: " + e.getMessage(), null);
        }
    }

    // Handles sending a message to the client
    private static void sendMessage() {
        String message = messageField.getText().trim();
        if (!message.isEmpty()) {
            String encodedMessage = huffman.encode(message); // Encode the message
            out.println(encodedMessage); // Send encoded message to client
            appendMessage("서버", message, encodedMessage); // Log the message in GUI
            messageField.setText(""); // Clear input field
            messageField.requestFocus(); // Focus back on input
        }
    }

    // Appends a message to the log area
    private static void appendMessage(String sender, String message, String encodedMessage) {
        SwingUtilities.invokeLater(() -> {
            StringBuilder sb = new StringBuilder();
            
            if (sender.equals("서버")) { // Message from server
                sb.append(String.format("\n                                                                                     보낸 메시지   <<<\n"));
                sb.append(String.format("%-10s %s\n", "메시지:", message));
                sb.append(String.format("%-10s %s\n", "인코딩:", encodedMessage));
                sb.append("─────────────────────────────────────\n");
            } else if (sender.equals("클라이언트")) { // Message from client
                sb.append("\n>>>   받은 메시지\n");
                sb.append(String.format("%-10s %s\n", "메시지:", message));
                sb.append(String.format("%-10s %s\n", "인코딩:", encodedMessage));
                sb.append("─────────────────────────────────────\n");
            } else { // System messages
                sb.append("\n[ " + message + " ]\n");
            }
            
            logArea.append(sb.toString());
            logArea.setCaretPosition(logArea.getDocument().getLength());
        });
    }

    // Handles receiving a message from the client
    private static void handleReceivedMessage(String encodedMessage) {
        String decodedMessage = huffman.decode(encodedMessage);
        appendMessage("클라이언트", decodedMessage, encodedMessage); // Log the decoded message
    }
}
