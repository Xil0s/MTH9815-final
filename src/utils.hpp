/**
 * utils.hpp
 *
 * This file provides utility functions and definitions to support various
 * operations across the financial data processing system.
 *
 * @author: Edgar Gonzalez
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <tuple>
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include "products.hpp"

/*
 * ProductMap
 * ----------
 * Provides static functions to return a bond map, a bond list, and a ticker list.
 * (B02y, B03y, B05y, B07y, B10y, B30y) are included for demonstration.
 */
class ProductMap
{
public:
    // Returns a map from "B02y"/etc. to a Bond object.
    static std::unordered_map<std::string, Bond> GetProductMap()
    {
        std::unordered_map<std::string, Bond> result;
        std::vector<Bond> bonds;

        // Basic six bond definitions
        bonds.push_back(Bond("B02y", CUSIP, "B02y", 0.02, date(2026, 12, 31)));
        bonds.push_back(Bond("B03y", CUSIP, "B03y", 0.025, date(2027, 12, 31)));
        bonds.push_back(Bond("B05y", CUSIP, "B05y", 0.03, date(2029, 12, 31)));
        bonds.push_back(Bond("B07y", CUSIP, "B07y", 0.035, date(2031, 12, 31)));
        bonds.push_back(Bond("B10y", CUSIP, "B10y", 0.04, date(2034, 12, 31)));
        bonds.push_back(Bond("B20y", CUSIP, "B20y", 0.045, date(2044, 12, 31)));
        bonds.push_back(Bond("B30y", CUSIP, "B30y", 0.05, date(2054, 12, 31)));

        // Populate map by product ID
        for(auto &b : bonds)
        {
            result.insert(std::make_pair(b.GetProductId(), b));
        }
        return result;
    }

    // Returns a list of Bond objects matching the above tickers
    static std::vector<Bond> GetProducts()
    {
        std::vector<Bond> bonds;
        bonds.push_back(Bond("B02y", CUSIP, "B02y", 0.02, date(2026, 12, 31)));
        bonds.push_back(Bond("B03y", CUSIP, "B03y", 0.025, date(2027, 12, 31)));
        bonds.push_back(Bond("B05y", CUSIP, "B05y", 0.03, date(2029, 12, 31)));
        bonds.push_back(Bond("B07y", CUSIP, "B07y", 0.035, date(2031, 12, 31)));
        bonds.push_back(Bond("B10y", CUSIP, "B10y", 0.04, date(2034, 12, 31)));
        bonds.push_back(Bond("B20y", CUSIP, "B20y", 0.045, date(2044, 12, 31)));
        bonds.push_back(Bond("B30y", CUSIP, "B30y", 0.05, date(2054, 12, 31)));
        return bonds;
    }

    // Returns the standard set of tickers
    static std::vector<std::string> GetTickers()
    {
        return {"B02y","B03y","B05y","B07y","B10y","B20y","B30y"};
    }
};

/*
 * FormatParser
 * ------------
 * Provides static functions to parse lines or values:
 *   - Parse(...) : splits text by ',' ignoring a character after each token
 *   - ParseCommaSepLine(...) : splits by ',' with no extra ignoring
 *   - ParsePriceFormat(...) : interprets a pseudo-bond-price string
 *   - ParseOrderBook(...) : returns (ticker, vector of 10 prices)
 */
class FormatParser
{
public:
    // Splits 'text' by ',', ignoring one extra char after each token
    static std::vector<std::string> Parse(const std::string &text)
    {
        std::vector<std::string> output;
        std::istringstream buffer(text);
        std::string segment;
        while (std::getline(buffer, segment, ','))
        {
            // ignore next char
            buffer.ignore();
            output.push_back(segment);
        }
        return output;
    }

    // Splits 'text' by ',' into tokens (no ignoring)
    static std::vector<std::string> ParseCommaSepLine(const std::string &text)
    {
        std::vector<std::string> output;
        std::istringstream buffer(text);
        std::string token;
        while (std::getline(buffer, token, ','))
        {
            output.push_back(token);
        }
        return output;
    }

    // Interprets price_string as a fractional bond price,
    // ex: "99-160" => ~ 99.5, or "100-25+" => slightly more, etc.
    // (Exact logic depends on your data format.)
    static double ParsePriceFormat(const std::string &price_string)
    {
        // Original style logic: if the first char is '9', then base=99, else base=100
        const char* cstr = price_string.c_str();
        int mainPart;
        int offset;
        if (cstr[0] == '9')
        {
            mainPart = 99;
            offset = 3;
        }
        else
        {
            mainPart = 100;
            offset = 4;
        }
        // parse next 3 digits
        int first = (cstr[offset]   - '0')*10 + (cstr[offset+1] - '0');
        int second = (cstr[offset+2] - '0');

        double fractional = first / 32.0 + second / 256.0;
        return mainPart + fractional;
    }

    // For a line describing market data with 1 ticker + 10 fractional prices,
    // returns (ticker, vector<double>)
    static std::tuple<std::string, std::vector<double>> ParseOrderBook(const std::string & text)
    {
        std::vector<std::string> splitted = ParseCommaSepLine(text);
        if (splitted.empty()) 
        {
            return std::make_tuple(std::string(), std::vector<double>());
        }
        std::string ticker = splitted[0];
        std::vector<double> px;
        px.reserve(10);

        // next 10 are fractional prices
        int count = (splitted.size() < 11) ? splitted.size()-1 : 10;
        for (int i = 1; i <= count; ++i)
        {
            px.push_back(ParsePriceFormat(splitted[i]));
        }
        return std::make_tuple(ticker, px);
    }
};

#endif // UTILS_HPP
