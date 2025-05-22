#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <cctype>

using namespace std;

// -------------------- Data Structures from original code --------------------
class ListNode {
public:
    string data;
    ListNode* next;
    ListNode(string value) : data(value), next(nullptr) {}
};

class FoodNode {
public:
    int foodNo;
    string name;
    double price;
    int inStock;
    string category;
    int totalSold;
    FoodNode* left;
    FoodNode* right;

    FoodNode(int number, string foodName, double foodPrice, int stock, string cat)
        : foodNo(number), name(foodName), price(foodPrice), inStock(stock),
        category(cat), totalSold(0), left(nullptr), right(nullptr) {
    }
};

class FoodManagementSystem {
private:
    FoodNode* root;
    ListNode* orderHistory;
    ListNode* adminLogs;
    double totalRevenue;

    FoodNode* insert(FoodNode* node, int number, string name, double price, int stock, string category) {
        if (node == nullptr) {
            return new FoodNode(number, name, price, stock, category);
        }
        if (number < node->foodNo) {
            node->left = insert(node->left, number, name, price, stock, category);
        }
        else if (number > node->foodNo) {
            node->right = insert(node->right, number, name, price, stock, category);
        }
        return node;
    }

    FoodNode* findFood(FoodNode* node, int number) {
        if (node == nullptr || node->foodNo == number) return node;
        if (number < node->foodNo) return findFood(node->left, number);
        return findFood(node->right, number);
    }

public:
    FoodManagementSystem() : root(nullptr), orderHistory(nullptr), adminLogs(nullptr), totalRevenue(0) {}

    void insertFood(int number, string name, double price, int stock, string category) {
        root = insert(root, number, name, price, stock, category);
        addAdminLog("Added Food Item: " + name + " (" + category + ")");
    }

    FoodNode* getRoot() { return root; }

    FoodNode* findFood(int number) {
        return findFood(root, number);
    }

    void addAdminLog(string logDetail) {
        ListNode* newLog = new ListNode(logDetail);
        newLog->next = adminLogs;
        adminLogs = newLog;
    }

    ListNode* getAdminLogs() { return adminLogs; }

    // In-order traversal to get all foods
    void getFoodList(FoodNode* node, vector<FoodNode*>& foods) {
        if (!node) return;
        getFoodList(node->left, foods);
        foods.push_back(node);
        getFoodList(node->right, foods);
    }

    vector<FoodNode*> getAllFoods() {
        vector<FoodNode*> foods;
        getFoodList(root, foods);
        return foods;
    }

    void processOrder(int orderNo, int quantity) {
        FoodNode* food = findFood(orderNo);
        if (food && food->inStock >= quantity) {
            double totalPrice = food->price * quantity;
            food->inStock -= quantity;
            food->totalSold += quantity;
            totalRevenue += totalPrice;
        }
    }

    bool validateCard(const string& cardNumber, const string& cardPassword) {
        // Very basic validation
        if (cardNumber.size() == 16 && !cardPassword.empty()) {
            return true;
        }
        return false;
    }

    bool updateFood(int number, string newName, double newPrice, int newStock, string newCategory) {
        FoodNode* food = findFood(root, number);
        if (!food) return false;
        food->name = newName;
        food->price = newPrice;
        food->inStock = newStock;
        food->category = newCategory;
        addAdminLog("Updated Food Item: " + newName + " (" + newCategory + ")");
        return true;
    }

    FoodNode* findMin(FoodNode* node) {
        while (node && node->left) node = node->left;
        return node;
    }

    FoodNode* deleteFood(FoodNode* node, int number) {
        if (!node) return nullptr;
        if (number < node->foodNo) {
            node->left = deleteFood(node->left, number);
        }
        else if (number > node->foodNo) {
            node->right = deleteFood(node->right, number);
        }
        else {
            // Node found
            if (!node->left) {
                FoodNode* temp = node->right;
                addAdminLog("Deleted Food Item: " + node->name);
                delete node;
                return temp;
            }
            else if (!node->right) {
                FoodNode* temp = node->left;
                addAdminLog("Deleted Food Item: " + node->name);
                delete node;
                return temp;
            }
            else {
                FoodNode* temp = findMin(node->right);
                node->foodNo = temp->foodNo;
                node->name = temp->name;
                node->price = temp->price;
                node->inStock = temp->inStock;
                node->category = temp->category;
                node->right = deleteFood(node->right, temp->foodNo);
            }
        }
        return node;
    }

    bool deleteFood(int number) {
        FoodNode* food = findFood(root, number);
        if (!food) return false;
        root = deleteFood(root, number);
        return true;
    }

};

// -------------------- Utility: Button Handling --------------------

struct Button {
    sf::RectangleShape shape;
    sf::Text text;

    bool isMouseOver(sf::RenderWindow& window) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        return shape.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }
};

Button createButton(sf::Font& font, const string& str, float x, float y, float width, float height) {
    Button btn;
    btn.shape.setSize(sf::Vector2f(width, height));
    btn.shape.setPosition(x, y);
    btn.shape.setFillColor(sf::Color::Blue);

    btn.text.setFont(font);
    btn.text.setString(str);
    btn.text.setCharacterSize(20);
    btn.text.setFillColor(sf::Color::White);
    sf::FloatRect bounds = btn.text.getLocalBounds();
    btn.text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    btn.text.setPosition(x + width / 2.0f, y + height / 2.0f);
    return btn;
}

bool handleButtonClick(Button& btn, sf::RenderWindow& window, sf::Event& event) {
    if (event.type == sf::Event::MouseButtonReleased &&
        event.mouseButton.button == sf::Mouse::Left &&
        btn.isMouseOver(window)) {
        return true;
    }
    return false;
}

// -------------------- States --------------------
enum class AppState {
    MainMenu,
    CustomerOrder,
    AdminLogin,
    AdminPanel,
    Exit
};

// -------------------- MainMenu State Handling --------------------
AppState mainMenuState(sf::RenderWindow& window, sf::Font& font) {
    Button customerBtn = createButton(font, "Food List & Order", 300, 200, 200, 50);
    Button adminBtn = createButton(font, "Admin Panel", 300, 300, 200, 50);
    Button exitBtn = createButton(font, "Exit", 300, 400, 200, 50);

    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("Food Ordering System");
    titleText.setCharacterSize(36);
    titleText.setFillColor(sf::Color::White);
    titleText.setStyle(sf::Text::Bold);

    // Centering the title text horizontally
    sf::FloatRect textRect = titleText.getLocalBounds();
    titleText.setOrigin(textRect.width / 2, textRect.height / 2);
    titleText.setPosition(window.getSize().x / 2, 60); // 60px from top

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return AppState::Exit;
            }

            if (handleButtonClick(customerBtn, window, event)) {
                return AppState::CustomerOrder;
            }
            if (handleButtonClick(adminBtn, window, event)) {
                return AppState::AdminLogin;
            }
            if (handleButtonClick(exitBtn, window, event)) {
                return AppState::Exit;
            }
        }

        window.clear(sf::Color::Black);
        window.draw(titleText);
        window.draw(customerBtn.shape);
        window.draw(customerBtn.text);
        window.draw(adminBtn.shape);
        window.draw(adminBtn.text);
        window.draw(exitBtn.shape);
        window.draw(exitBtn.text);
        window.display();
    }
    return AppState::Exit;
}

// -------------------- CustomerOrder State Handling --------------------
AppState customerOrderState(sf::RenderWindow& window, sf::Font& font, FoodManagementSystem& fms) {
    vector<FoodNode*> foods = fms.getAllFoods();

    Button backBtn = createButton(font, "Back", 50, 500, 100, 40);
    Button orderBtn = createButton(font, "Place Order", 600, 500, 150, 50);

    // Input fields variables
    string inputFoodNo = "";
    string inputQuantity = "";
    bool inputFoodNoActive = false;
    bool inputQuantityActive = false;

    sf::RectangleShape foodNoBox(sf::Vector2f(150, 30));
    foodNoBox.setPosition(50, 450);
    foodNoBox.setFillColor(sf::Color::White);

    sf::RectangleShape quantityBox(sf::Vector2f(150, 30));
    quantityBox.setPosition(250, 450);
    quantityBox.setFillColor(sf::Color::White);

    sf::Text foodNoLabel("Food No:", font, 20);
    foodNoLabel.setPosition(50, 420);
    sf::Text quantityLabel("Quantity:", font, 20);
    quantityLabel.setPosition(250, 420);

    sf::Text inputFoodNoText("", font, 20);
    inputFoodNoText.setPosition(55, 455);
    inputFoodNoText.setFillColor(sf::Color::Black);

    sf::Text inputQuantityText("", font, 20);
    inputQuantityText.setPosition(255, 455);
    inputQuantityText.setFillColor(sf::Color::Black);

    sf::Text messageText("", font, 20);
    messageText.setPosition(50, 520);
    messageText.setFillColor(sf::Color::Green);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                return AppState::Exit;

            if (handleButtonClick(backBtn, window, event))
                return AppState::MainMenu;

            if (handleButtonClick(orderBtn, window, event)) {
                // Validate and process order
                if (inputFoodNo.empty() || inputQuantity.empty()) {
                    messageText.setString("Please enter both Food No and Quantity.");
                    messageText.setFillColor(sf::Color::Red);
                }
                else {
                    int foodNo = stoi(inputFoodNo);
                    int quantity = stoi(inputQuantity);
                    FoodNode* food = fms.findFood(foodNo);
                    if (!food) {
                        messageText.setString("Food item not found.");
                        messageText.setFillColor(sf::Color::Red);
                    }
                    else if (quantity <= 0) {
                        messageText.setString("Quantity must be positive.");
                        messageText.setFillColor(sf::Color::Red);
                    }
                    else if (food->inStock < quantity) {
                        messageText.setString("Insufficient stock.");
                        messageText.setFillColor(sf::Color::Red);
                    }
                    else {
                        fms.processOrder(foodNo, quantity);
                        messageText.setString("Order placed: " + to_string(quantity) + " x " + food->name);
                        messageText.setFillColor(sf::Color::Green);

                        // Clear inputs after success
                        inputFoodNo.clear();
                        inputQuantity.clear();
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                // Check which input box is active
                if (foodNoBox.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    inputFoodNoActive = true;
                    inputQuantityActive = false;
                }
                else if (quantityBox.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    inputFoodNoActive = false;
                    inputQuantityActive = true;
                }
                else {
                    inputFoodNoActive = false;
                    inputQuantityActive = false;
                }
            }

            if (event.type == sf::Event::TextEntered) {
                if (inputFoodNoActive) {
                    if (isdigit(event.text.unicode) && inputFoodNo.size() < 5) {
                        inputFoodNo += static_cast<char>(event.text.unicode);
                    }
                    else if (event.text.unicode == 8 && !inputFoodNo.empty()) { // backspace
                        inputFoodNo.pop_back();
                    }
                }
                else if (inputQuantityActive) {
                    if (isdigit(event.text.unicode) && inputQuantity.size() < 3) {
                        inputQuantity += static_cast<char>(event.text.unicode);
                    }
                    else if (event.text.unicode == 8 && !inputQuantity.empty()) { // backspace
                        inputQuantity.pop_back();
                    }
                }
            }
        }

        // Update input text display
        inputFoodNoText.setString(inputFoodNo);
        inputQuantityText.setString(inputQuantity);

        window.clear(sf::Color::Black);

        // Draw food list
        float startY = 50;
        for (auto* food : foods) {
            sf::Text t;
            t.setFont(font);
            t.setString(to_string(food->foodNo) + ". " + food->name + " $" + to_string(food->price) + " (In stock: " + to_string(food->inStock) + ")");
            t.setCharacterSize(20);
            t.setPosition(50, startY);
            startY += 30;
            window.draw(t);
        }

        // Draw UI elements
        window.draw(foodNoLabel);
        window.draw(quantityLabel);
        window.draw(foodNoBox);
        window.draw(quantityBox);
        window.draw(inputFoodNoText);
        window.draw(inputQuantityText);
        window.draw(orderBtn.shape);
        window.draw(orderBtn.text);
        window.draw(backBtn.shape);
        window.draw(backBtn.text);
        window.draw(messageText);

        window.display();
    }

    return AppState::MainMenu;
}


            //Modified AdminLoginState

AppState adminLoginState(sf::RenderWindow& window, sf::Font& font, bool& adminLoggedIn) {
    // Admin credentials
    const string ADMIN_USER = "admin";
    const string ADMIN_PASS = "pass";

    // UI Elements
    Button backBtn = createButton(font, "Back", 50, 500, 100, 40);

    sf::Text prompt;
    prompt.setFont(font);
    prompt.setCharacterSize(24);
    prompt.setString("Enter admin username and password:");
    prompt.setPosition(50, 50);

    sf::Text userLabel, passLabel;
    userLabel.setFont(font);
    userLabel.setCharacterSize(20);
    userLabel.setString("Username:");
    userLabel.setPosition(50, 120);

    passLabel.setFont(font);
    passLabel.setCharacterSize(20);
    passLabel.setString("Password:");
    passLabel.setPosition(50, 200);

    string username, password;
    bool enteringUsername = true;
    bool showError = false;

    sf::Text errorText;
    errorText.setFont(font);
    errorText.setCharacterSize(20);
    errorText.setFillColor(sf::Color::Red);
    errorText.setPosition(50, 300);
    errorText.setString("Invalid credentials. Try again.");

    sf::Text userInputText, passInputText;
    userInputText.setFont(font);
    userInputText.setCharacterSize(20);
    userInputText.setPosition(200, 120);

    passInputText.setFont(font);
    passInputText.setCharacterSize(20);
    passInputText.setPosition(200, 200);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                return AppState::Exit;

            if (handleButtonClick(backBtn, window, event))
                return AppState::MainMenu;

            if (event.type == sf::Event::TextEntered) {
                char inputChar = static_cast<char>(event.text.unicode);
                if (event.text.unicode == 8) { // Backspace
                    if (enteringUsername && !username.empty())
                        username.pop_back();
                    else if (!enteringUsername && !password.empty())
                        password.pop_back();
                }
                else if (event.text.unicode == 13) { // Enter
                    if (enteringUsername)
                        enteringUsername = false;
                    else {
                        // Attempt login
                        if (username == ADMIN_USER && password == ADMIN_PASS) {
                            adminLoggedIn = true;
                            return AppState::AdminPanel;
                        }
                        else {
                            showError = true;
                            password.clear();
                        }
                    }
                }
                else if (event.text.unicode >= 32 && event.text.unicode < 127) {
                    if (enteringUsername)
                        username += inputChar;
                    else
                        password += inputChar;
                }
            }
        }

        // Update UI
        userInputText.setString(username);
        passInputText.setString(string(password.size(), '*')); // Hide password characters

        window.clear(sf::Color::Black);
        window.draw(prompt);
        window.draw(userLabel);
        window.draw(userInputText);
        window.draw(passLabel);
        window.draw(passInputText);
        window.draw(backBtn.shape);
        window.draw(backBtn.text);
        if (showError) window.draw(errorText);
        window.display();
    }

    return AppState::MainMenu;
}


// -------------------- AdminPanel State Handling --------------------
AppState adminPanelState(sf::RenderWindow& window, sf::Font& font, FoodManagementSystem& fms) {
    Button backBtn = createButton(font, "Back", 50, 500, 100, 40);
    Button addBtn = createButton(font, "Add Food", 50, 420, 150, 40);
    Button updateBtn = createButton(font, "Update Food", 220, 420, 150, 40);
    Button deleteBtn = createButton(font, "Delete Food", 390, 420, 150, 40);

    string inputFoodNo = "";
    string inputName = "";
    string inputPrice = "";
    string inputStock = "";
    string inputCategory = "";


    sf::Text messageText("", font, 20);
    messageText.setPosition(50, 460);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                return AppState::Exit;

            if (handleButtonClick(backBtn, window, event))
                return AppState::MainMenu;

            if (handleButtonClick(addBtn, window, event)) {
                try {
                    int number = stoi(inputFoodNo);
                    double price = stod(inputPrice);
                    int stock = stoi(inputStock);
                    if (inputName.empty() || inputCategory.empty()) {
                        messageText.setString("Name and Category cannot be empty");
                        messageText.setFillColor(sf::Color::Red);
                    }
                    else {
                        fms.insertFood(number, inputName, price, stock, inputCategory);
                        messageText.setString("Food added successfully!");
                        messageText.setFillColor(sf::Color::Green);
                    }
                }
                catch (...) {
                    messageText.setString("Invalid input");
                    messageText.setFillColor(sf::Color::Red);
                }
            }

            if (handleButtonClick(updateBtn, window, event)) {
                try {
                    int number = stoi(inputFoodNo);
                    double price = stod(inputPrice);
                    int stock = stoi(inputStock);
                    if (inputName.empty() || inputCategory.empty()) {
                        messageText.setString("Name and Category cannot be empty");
                        messageText.setFillColor(sf::Color::Red);
                    }
                    else {
                        if (fms.updateFood(number, inputName, price, stock, inputCategory))
                            messageText.setString("Food updated successfully!");
                        else
                            messageText.setString("Food not found.");
                        messageText.setFillColor(sf::Color::Green);
                    }
                }
                catch (...) {
                    messageText.setString("Invalid input");
                    messageText.setFillColor(sf::Color::Red);
                }
            }

            if (handleButtonClick(deleteBtn, window, event)) {
                try {
                    int number = stoi(inputFoodNo);
                    if (fms.deleteFood(number))
                        messageText.setString("Food deleted successfully!");
                    else
                        messageText.setString("Food not found.");
                    messageText.setFillColor(sf::Color::Green);
                }
                catch (...) {
                    messageText.setString("Invalid input");
                    messageText.setFillColor(sf::Color::Red);
                }
            }
        }

        window.clear(sf::Color::Black);
        window.draw(messageText);
        window.draw(backBtn.shape);
        window.draw(backBtn.text);
        window.draw(addBtn.shape);
        window.draw(addBtn.text);
        window.draw(updateBtn.shape);
        window.draw(updateBtn.text);
        window.draw(deleteBtn.shape);
        window.draw(deleteBtn.text);

        window.display();
    }
    return AppState::MainMenu;
}

// -------------------- Main --------------------
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Food Ordering System");
    sf::Font font;
    if (!font.loadFromFile("constan.ttf")) {
        cout << "Failed to load font!" << endl;
        return -1;
    }

    FoodManagementSystem fms;

    fms.insertFood(1, "Burger", 5.99, 10, "Fast Food");
    fms.insertFood(2, "Pizza", 8.99, 8, "Fast Food");
    fms.insertFood(3, "Pasta", 6.49, 25, "Main Course");
    fms.insertFood(4, "Ice Cream", 3.99, 30, "Desserts");
    fms.insertFood(5, "Salad", 4.99, 15, "Healthy");

    AppState state = AppState::MainMenu;
    bool adminLoggedIn = false;

    while (window.isOpen()) {
        switch (state) {
        case AppState::MainMenu:
            state = mainMenuState(window, font);
            break;

        case AppState::CustomerOrder:
            state = customerOrderState(window, font, fms);
            break;

        case AppState::AdminLogin:
            state = adminLoginState(window, font, adminLoggedIn);
            break;

        case AppState::AdminPanel:
            if (adminLoggedIn) {
                state = adminPanelState(window, font, fms);
            }
            else {
                state = AppState::AdminLogin;
            }
            break;

        case AppState::Exit:
            window.close();
            break;
        }
    }

    return 0;
}