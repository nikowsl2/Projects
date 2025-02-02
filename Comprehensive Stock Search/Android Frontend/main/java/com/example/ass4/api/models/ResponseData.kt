package com.example.ass4.api.models


data class ResponseData(
    val companyData: CompanyData,
    val companyQuote: CompanyQuote,
    val recommendationTrend: List<RecommendationTrendItem>,
    val companyEarnings: List<CompanyEarnings>,
    val companyInsider: List<CompanyInsider>,
    val companyPeer: List<String>,
    val newsSet: List<NewsItem>,
    val tradeSet: List<TradeItem>,
    val hourlyTradeSet: List<HourlyTradeItem>,
    val lastOpenDate: String
)

data class CompanyData(
    val Ticker: String,
    val CompanysName: String,
    val ExchangeCode: String,
    val IPOStartDate: String,
    val Industry: String,
    val Webpage: String,
    val Logo: String
)

data class CompanyQuote(
    val LastPrice: Double,
    val Change: Double,
    val ChangePercentage: Double,
    val CurrentTimestamp: Long,
    val HighPrice: Double,
    val LowPrice: Double,
    val OpenPrice: Double,
    val PrevClose: Double,
    val TimeStamp: Long
)

data class RecommendationTrendItem(
    val Buy: Int,
    val Hold: Int,
    val period: String,
    val sell: Int,
    val strongBuy: Int,
    val strongSell: Int,
    val symbol: String
)

data class CompanyEarnings(
    val actual: Double,
    val estimate: Double,
    val period: String,
    val symbol: String,
    val surprise: Double
)

data class CompanyInsider(
    val symbol: String,
    val change: Double,
    val mspr: Double
)

data class NewsItem(
    val Image: String,
    val Title: String,
    val Description: String,
    val LinkToOriginalPost: String,
    val Source: String,
    val PublishedDate: Long
)

data class TradeItem(
    val Date: Long,
    val StockPrice: Double,
    val Volume: Int,
    val Open: Double,
    val High: Double,
    val Low: Double,
    val Close: Double
)

data class HourlyTradeItem(
    val Date: Long,
    val StockPrice: Double,
    val Volume: Int
)

