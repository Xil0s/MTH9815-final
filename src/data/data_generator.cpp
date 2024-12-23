#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <tuple>

// Define the specific bond tickers as per your ProductMap
const std::vector<std::string> BOND_TICKERS = {"B02y", "B03y", "B05y", "B07y", "B10y", "B20y", "B30y"};

// Define possible trade books
const std::vector<std::string> TRADE_BOOKS = {"TRSY1", "TRSY2", "TRSY3"};

// Define possible trade sides
const std::vector<std::string> TRADE_SIDES = {"BUY", "SELL"};

// Define possible inquiry statuses
const std::vector<std::string> INQUIRY_STATUSES = {"RECEIVED", "PROCESSING", "COMPLETED", "CANCELLED"};

// Define possible price types
const std::vector<std::string> PRICE_TYPES = {"BID", "ASK"};

// Define possible market data statuses
const std::vector<std::string> MARKETDATA_STATUSES = {"OPEN", "CLOSED", "PENDING"};

/**
 * @brief Generates a random integer within a specified range.
 *
 * @param min Minimum value (inclusive).
 * @param max Maximum value (inclusive).
 * @param rng Random number generator.
 * @return int Random integer between min and max.
 */
int getRandomInt(int min, int max, std::mt19937 &rng) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

/**
 * @brief Generates a random double within a specified range.
 *
 * @param min Minimum value (inclusive).
 * @param max Maximum value (inclusive).
 * @param rng Random number generator.
 * @return double Random double between min and max.
 */
double getRandomDouble(double min, double max, std::mt19937 &rng) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

/**
 * @brief Generates a random price string in the format "99-16+" or "100-05".
 *
 * The format is:
 * - Whole number (e.g., "99" or "100")
 * - Dash "-"
 * - Two-digit fractional part (e.g., "16" representing 16/32)
 * - Optional "+" indicating an additional half of 1/32 (i.e., 1/64)
 *
 * @param rng Random number generator.
 * @return std::string Randomly generated price string.
 */
std::string generateFractionalPrice(std::mt19937 &rng) {
    // Determine whole number part
    std::uniform_int_distribution<int> wholeDist(99, 100);
    int whole = wholeDist(rng);
    
    // Determine fractional part (0 to 31)
    std::uniform_int_distribution<int> fracDist(0, 31);
    int frac = fracDist(rng);
    
    // Determine if a '+' should be appended
    std::bernoulli_distribution plusDist(0.3); // 30% chance of '+'
    bool hasPlus = plusDist(rng);
    
    // Format fractional part with leading zeros if necessary
    std::string fracStr = (frac < 10) ? "0" + std::to_string(frac) : std::to_string(frac);
    
    // Construct price string
    std::string price = std::to_string(whole) + "-" + fracStr;
    if (hasPlus) {
        price += "+";
    }
    
    return price;
}

/**
 * @brief Creates the trades.txt file with randomly generated trade entries.
 *
 * Each trade entry consists of:
 * - Ticker
 * - Trade ID
 * - Book
 * - Quantity
 * - Price
 * - Side
 *
 * @param filename Name of the output trades file.
 * @param numTrades Number of trade entries to generate.
 */
void createTradesFile(const std::string &filename, int numTrades) {
    std::ofstream tradesFile(filename);
    if (!tradesFile.is_open()) {
        std::cerr << "Error: Unable to open " << filename << " for writing.\n";
        return;
    }

    // Initialize random number generator with a seed based on current time
    std::mt19937 rng(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));

    for (int i = 1; i <= numTrades; ++i) {
        // Select random ticker, book, and side
        std::string ticker = BOND_TICKERS[getRandomInt(0, BOND_TICKERS.size() - 1, rng)];
        std::string tradeId = "TradeId" + std::to_string(i);
        std::string book = TRADE_BOOKS[getRandomInt(0, TRADE_BOOKS.size() - 1, rng)];
        long quantity = getRandomInt(1, 5, rng) * 1000000; // Quantities: 1,000,000 to 5,000,000
        std::string price = generateFractionalPrice(rng);     // Fractional price
        std::string side = TRADE_SIDES[getRandomInt(0, TRADE_SIDES.size() - 1, rng)];

        // Write the trade entry to the file
        tradesFile << ticker << "," << tradeId << "," << book << "," << quantity << "," << price << "," << side << "\n";

        // Optional: Display progress for every 10 trades
        if (i % 10 == 0) {
            std::cout << "Generated " << i << " trades...\n";
        }
    }

    tradesFile.close();
    std::cout << "Successfully created " << filename << " with " << numTrades << " trades.\n";
}

/**
 * @brief Creates the inquiries.txt file with randomly generated inquiry entries.
 *
 * Each inquiry entry consists of:
 * - Inquiry ID
 * - Ticker
 * - Side
 * - Status
 *
 * @param filename Name of the output inquiries file.
 * @param numInquiries Number of inquiry entries to generate.
 */
void createInquiriesFile(const std::string &filename, int numInquiries) {
    std::ofstream inquiriesFile(filename);
    if (!inquiriesFile.is_open()) {
        std::cerr << "Error: Unable to open " << filename << " for writing.\n";
        return;
    }

    // Initialize random number generator with a different seed
    std::mt19937 rng(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));

    for (int i = 1; i <= numInquiries; ++i) {
        // Select random ticker, side, and status
        std::string ticker = BOND_TICKERS[getRandomInt(0, BOND_TICKERS.size() - 1, rng)];
        std::string inquiryId = std::to_string(i);
        std::string side = TRADE_SIDES[getRandomInt(0, TRADE_SIDES.size() - 1, rng)];
        std::string status = INQUIRY_STATUSES[getRandomInt(0, INQUIRY_STATUSES.size() - 1, rng)];

        // Write the inquiry entry to the file
        inquiriesFile << inquiryId << "," << ticker << "," << side << "," << status << "\n";

        // Optional: Display progress for every 10 inquiries
        if (i % 10 == 0) {
            std::cout << "Generated " << i << " inquiries...\n";
        }
    }

    inquiriesFile.close();
    std::cout << "Successfully created " << filename << " with " << numInquiries << " inquiries.\n";
}

/**
 * @brief Creates the prices.txt file with randomly generated price entries.
 *
 * Each price entry consists of:
 * - Ticker
 * - Bid Price
 * - Ask Price
 *
 * @param filename Name of the output prices file.
 * @param numPrices Number of price entries to generate.
 */
void createPricesFile(const std::string &filename, int numPrices) {
    std::ofstream pricesFile(filename);
    if (!pricesFile.is_open()) {
        std::cerr << "Error: Unable to open " << filename << " for writing.\n";
        return;
    }

    // Initialize random number generator
    std::mt19937 rng(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()) + 1);

    for (int i = 1; i <= numPrices; ++i) {
        // Select random ticker
        std::string ticker = BOND_TICKERS[getRandomInt(0, BOND_TICKERS.size() - 1, rng)];

        // Generate Bid and Ask prices ensuring Bid < Ask
        double bidDecimal = getRandomDouble(98.0, 100.0, rng);
        double askDecimal = getRandomDouble(bidDecimal + 0.1, 102.0, rng); // Ensure Ask > Bid by at least 0.1

        // Convert decimal prices back to fractional format
        auto decimalToFractional = [&](double price) -> std::string {
            int whole = static_cast<int>(price);
            double frac = price - whole;
            int thirtySeconds = static_cast<int>(frac * 32);
            double remainder = frac * 32 - thirtySeconds;
            bool hasPlus = remainder > 0.25; // If remainder > 0.25 (i.e., more than half of 1/32)

            std::string fracStr = (thirtySeconds < 10) ? "0" + std::to_string(thirtySeconds) : std::to_string(thirtySeconds);
            if (hasPlus) {
                fracStr += "+";
            }
            return std::to_string(whole) + "-" + fracStr;
        };

        std::string bid = decimalToFractional(bidDecimal);
        std::string ask = decimalToFractional(askDecimal);

        // Write the price entry to the file
        pricesFile << ticker << "," << bid << "," << ask << "\n";

        // Optional: Display progress for every 10 prices
        if (i % 10 == 0) {
            std::cout << "Generated " << i << " price entries...\n";
        }
    }

    pricesFile.close();
    std::cout << "Successfully created " << filename << " with " << numPrices << " price entries.\n";
}

/**
 * @brief Creates the marketdata.txt file with randomly generated market data entries.
 *
 * Each market data entry consists of:
 * - Ticker
 * - Multiple Bid and Ask Prices (e.g., 5 levels each)
 *
 * The format is:
 * Ticker,Bid1,Ask1,Bid2,Ask2,Bid3,Ask3,Bid4,Ask4,Bid5,Ask5
 *
 * @param filename Name of the output market data file.
 * @param numEntries Number of market data entries to generate.
 */
void createMarketDataFile(const std::string &filename, int numEntries) {
    std::ofstream marketDataFile(filename);
    if (!marketDataFile.is_open()) {
        std::cerr << "Error: Unable to open " << filename << " for writing.\n";
        return;
    }

    // Initialize random number generator
    std::mt19937 rng(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()) + 2);

    for (int i = 1; i <= numEntries; ++i) {
        // Select random ticker
        std::string ticker = BOND_TICKERS[getRandomInt(0, BOND_TICKERS.size() - 1, rng)];

        std::vector<std::string> bidsAsks;
        bidsAsks.reserve(10); // 5 bids and 5 asks

        // Generate 5 levels of Bid and Ask prices
        for (int level = 0; level < 5; ++level) {
            // Generate Bid price
            double bidDecimal = getRandomDouble(98.0, 100.0, rng);
            // Generate Ask price ensuring Ask > Bid
            double askDecimal = getRandomDouble(bidDecimal + 0.1, 102.0, rng);

            // Convert decimal prices back to fractional format
            auto decimalToFractional = [&](double price) -> std::string {
                int whole = static_cast<int>(price);
                double frac = price - whole;
                int thirtySeconds = static_cast<int>(frac * 32);
                double remainder = frac * 32 - thirtySeconds;
                bool hasPlus = remainder > 0.25; // If remainder > 0.25 (i.e., more than half of 1/32)

                std::string fracStr = (thirtySeconds < 10) ? "0" + std::to_string(thirtySeconds) : std::to_string(thirtySeconds);
                if (hasPlus) {
                    fracStr += "+";
                }
                return std::to_string(whole) + "-" + fracStr;
            };

            std::string bid = decimalToFractional(bidDecimal);
            std::string ask = decimalToFractional(askDecimal);

            bidsAsks.push_back(bid);
            bidsAsks.push_back(ask);
        }

        // Write the market data entry to the file
        marketDataFile << ticker;
        for (const auto &price : bidsAsks) {
            marketDataFile << "," << price;
        }
        marketDataFile << "\n";

        // Optional: Display progress for every 10 market data entries
        if (i % 10 == 0) {
            std::cout << "Generated " << i << " market data entries...\n";
        }
    }

    marketDataFile.close();
    std::cout << "Successfully created " << filename << " with " << numEntries << " market data entries.\n";
}

int main() {
    // Define the number of records to generate for each file
    const int TOTAL_TRADES = 60;
    const int TOTAL_INQUIRIES = 60;
    const int TOTAL_PRICES = 60;
    const int TOTAL_MARKETDATA = 60;

    std::cout << "Initiating data generation process...\n\n";

    // Generate trades.txt
    createTradesFile("trades.txt", TOTAL_TRADES);

    // Generate inquiries.txt
    createInquiriesFile("inquiries.txt", TOTAL_INQUIRIES);

    // Generate prices.txt
    createPricesFile("prices.txt", TOTAL_PRICES);

    // Generate marketdata.txt
    createMarketDataFile("marketdata.txt", TOTAL_MARKETDATA);

    std::cout << "\nData generation completed successfully.\n";
    return 0;
}
