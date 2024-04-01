require('dotenv').config();
const express = require('express');
const axios = require('axios');
const cors = require('cors');
const app = express();
const PORT = process.env.PORT || 8080;
const path = require('path');


const { MongoClient, ServerApiVersion } = require('mongodb');
const uri = "mongodb+srv://nikowslcs571:4804qwer@cs571.orpjyv7.mongodb.net/?retryWrites=true&w=majority&appName=CS571";
const client = new MongoClient(uri, {
    serverApi: {
        version: ServerApiVersion.v1,
        strict: true,
        deprecationErrors: true,
    }
});

app.use(cors());
// app.use(express.static(path.join(__dirname, '../build')));

// app.use(cors({
//     origin: 'https://wsl571-assignment3.wl.r.appspot.com',
//     methods: ['GET', 'POST', 'OPTIONS'],
//     allowedHeaders: ['Content-Type', 'Authorization'],
//     credentials: true,
// }));

app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    res.header('Access-Control-Allow-Headers', '*');
    next();
});


app.use(express.json());

async function main() {
    try {
        await client.connect();
        console.log("Connected to MongoDB");
        const balance = client.db("Stock").collection("Balance");
        const documents = await balance.find().toArray();
        const portfolio = client.db("Stock").collection("Portfolio");
        const documents2 = await portfolio.find().toArray();
        console.log('balance!', documents);
        console.log('Potfolio!', documents2);
    }
    catch (error) {
        console.error("Unable to connect to MongoDB", error);
    }
}
main().catch(console.dir);

app.get('/api/balance/search', async (req, res) => {
    try {
        const database = client.db("Stock");
        const balanceCollection = database.collection("Balance");
        const balanceDocument = await balanceCollection.findOne({});

        if (!balanceDocument) {
            return res.status(404).send("Balance not found");
        }

        res.status(200).json(balanceDocument);
    } catch (error) {
        console.error("Error fetching balance from MongoDB:", error);
        res.status(500).send("Failed to fetch balance from MongoDB.");
    }
});

app.post('/api/balance/update', async (req, res) => {
    console.log("Reached /api/balance/update endpoint");
    try {
        const database = client.db("Stock");
        const balanceCollection = database.collection("Balance");
        const { balance } = req.body;

        if (balance === undefined || typeof balance !== 'number') {
            return res.status(400).send("Invalid or missing 'balance' parameter in request body.");
        }

        const filter = {};
        const options = { upsert: true };
        const updateDoc = {
            $set: {
                balance: balance
            }
        };

        const result = await balanceCollection.updateOne(filter, updateDoc, options);

        if (result.matchedCount === 0 && result.upsertedCount === 0) {
            res.status(404).send("Balance document not found and not inserted.");
        } else if (result.matchedCount === 1 && result.modifiedCount === 0) {
            res.status(200).send("Balance document matched but already has the new balance value.");
        } else {
            res.status(200).send("Balance updated successfully.");
        }
    }
    catch (error) {
        console.error("Error updating balance in MongoDB:", error);
        res.status(500).send("Failed to update balance in MongoDB.");
    }
});

app.get('/api/portfolio/search', async (req, res) => {
    try {
        const companyTicker = req.query.ticker;
        console.log('searchportfolio!', companyTicker);
        const database = client.db("Stock");
        const portfolioCollection = database.collection("Portfolio");
        const query = { ticker: companyTicker };
        const portfolioDocuments = await portfolioCollection.find(query).toArray();

        if (portfolioDocuments.length === 0) {
            return res.status(404).send(`No documents found for ticker ${companyTicker}`);
        }

        res.status(200).json(portfolioDocuments);
    }
    catch (error) {
        console.error("Error fetching documents from MongoDB:", error);
        res.status(500).send("Failed to fetch documents from MongoDB.");
    }
});

app.get('/api/portfolio/tickers', async (req, res) => {
    try {
        const database = client.db("Stock");
        const portfolioCollection = database.collection("Portfolio");
        const tickers = await portfolioCollection.aggregate([
            { $group: { _id: "$ticker" } }
        ]).toArray();
        console.log('tickers!', tickers);
        // if (tickers.length === 0) {
        //     return res.status(404).send("No tickers found");
        // }
        if (tickers.length === 0) {
            return res.status(200).json([]);
        }

        res.status(200).json(tickers);
    }
    catch (error) {
        console.error("Error fetching distinct tickers from MongoDB:", error);
        res.status(500).send("Failed to fetch distinct tickers from MongoDB.");
    }
});

app.post('/api/portfolio/purchase', async (req, res) => {
    try {
        const { companyTicker, companyName, numberOfShares, sharePrice } = req.body;
        const parsedNumberOfShares = parseInt(numberOfShares, 10);
        const parsedSharePrice = parseFloat(sharePrice);

        if (!companyTicker || !companyName || isNaN(parsedNumberOfShares) || isNaN(parsedSharePrice)) {
            return res.status(400).send("Missing or invalid parameters in the request body.");
        }

        const totalCost = parsedNumberOfShares * parsedSharePrice;

        console.log('insertPortfolio!', companyTicker);

        const database = client.db("Stock");
        const portfolioCollection = database.collection("Portfolio");
        const insertResult = await portfolioCollection.insertOne({
            ticker: companyTicker,
            name: companyName,
            shares: parsedNumberOfShares,
            purchasePrice: parsedSharePrice,
            cumulativeCost: totalCost
        });

        res.status(201).json({ message: `Document with ID ${insertResult.insertedId} inserted` });

    }
    catch (error) {
        console.error("Error inserting document into Portfolio:", error);
        res.status(500).json({ error: "Failed to insert document into Portfolio.", details: error.message });
    }
});

app.post('/api/portfolio/sell', async (req, res) => {
    const { ticker, sellPrice, numberOfSharesToSell } = req.body;
    const sellPriceFloat = parseFloat(sellPrice);
    const sharesToSell = parseInt(numberOfSharesToSell, 10);

    try {
        const database = client.db("Stock");
        const portfolioCollection = database.collection("Portfolio");
        let remainingSharesToSell = sharesToSell;
        let totalSaleValue = 0;
        const stocks = await portfolioCollection.find(
            { ticker: ticker })
            .sort({ _id: 1 })
            .toArray();

        for (const stock of stocks) {
            if (remainingSharesToSell <= 0) break;
            const sharesFromThisStock = Math.min(stock.shares, remainingSharesToSell);
            remainingSharesToSell -= sharesFromThisStock;
            totalSaleValue += sharesFromThisStock * sellPriceFloat;

            if (stock.shares > sharesFromThisStock) {
                await portfolioCollection.updateOne(
                    { _id: stock._id },
                    { $inc: { shares: -sharesFromThisStock } }
                );
            }
            else {
                await portfolioCollection.deleteOne({ _id: stock._id });
            }
        }

        if (remainingSharesToSell > 0) {
            return res.status(400).json({ message: "Not enough shares to sell." });
        }

        res.status(200).json({ message: "Stocks sold successfully.", totalSaleValue });
    }
    catch (error) {
        console.error("Error selling stock:", error);
        res.status(500).json({ error: "Failed to sell stock.", details: error.message });
    }
});

app.get('/api/watchlist/getList', async (req, res) => {
    try {
        const watchlist = client.db("Stock").collection("Watchlist");
        const documents = await watchlist.find().toArray();
        res.status(200).json(documents);
    }
    catch (error) {
        console.error("Error fetching watchlist from MongoDB:", error);
        res.status(500).send("Failed to fetch watchlist from MongoDB.");
    }
});

app.get('/api/watchlist/search', async (req, res) => {
    try {
        const companyTicker = req.query.ticker;
        console.log('searchMongo!', companyTicker);
        const database = client.db("Stock");
        const collection = database.collection("Watchlist");
        const query = { ticker: companyTicker };
        const result = await collection.findOne(query);

        if (result) {
            res.status(200).json({ found: true, data: result });
        }
        else {
            res.status(404).json({ found: false });
        }
    }
    catch (error) {
        res.status(500).send(error.message);
    }
});

app.post('/api/watchlist/insert', async (req, res) => {
    try {
        const { ticker: companyTicker } = req.body;
        console.log('insertmongo!', companyTicker);
        const database = client.db("Stock");
        const collection = database.collection("Watchlist");
        const upsertResult = await collection.updateOne(
            { ticker: companyTicker },
            { $setOnInsert: { ticker: companyTicker } },
            { upsert: true }
        );

        if (upsertResult.upsertedCount > 0) {
            console.log(`A document was inserted with the _id: ${upsertResult.upsertedId._id}`);
            res.status(200).json({ message: `Ticker ${companyTicker} inserted successfully.`, _id: upsertResult.upsertedId._id });
        }
        else {
            console.log(`Document with ticker "${companyTicker}" already exists.`);
            res.status(200).json({ message: `Document with ticker "${companyTicker}" already exists.` });
        }
    }
    catch (error) {
        console.error(error);
        res.status(500).send(error.message);
    }
});

app.delete('/api/watchlist/delete', async (req, res) => {
    try {
        const companyTicker = req.query.ticker;
        console.log('deletemongo!', companyTicker);
        const database = client.db("Stock");
        const collection = database.collection("Watchlist");
        const deletedDocument = await collection.findOneAndDelete({ ticker: companyTicker });

        console.log("Deleted document value:", deletedDocument);
        if (deletedDocument) {
            console.log(`Deleted document ticker:`, deletedDocument.ticker);
            res.status(200).json({ message: `Ticker ${companyTicker} successfully deleted.`, deletedDocument });
        } else {
            console.log(`No document found with ticker: ${companyTicker}`);
            res.status(404).json({ message: `No document found with ticker: ${companyTicker}.` });
        }
    } catch (error) {
        console.error("Error deleting document:", error);
        res.status(500).send("Failed to delete document.");
    }
});

// app.delete('/api/watchlist/delete', async (req, res) => {
//     try {
//         const { ticker: companyTicker } = req.body;
//         console.log('deletemongo!', companyTicker);
//         const database = client.db("Stock");
//         const collection = database.collection("Watchlist");
//         const deletedDocument = await collection.findOneAndDelete({ ticker: companyTicker });
//         console.log("Deleted document value:", deletedDocument.value);
//         console.log("Full response:", deletedDocument);
//         if (deletedDocument) {
//             console.log(`Deleted document:`, deletedDocument.ticker);
//             res.status(200).json({ message: `Ticker ${companyTicker} successfully deleted.`, deletedDocument: deletedDocument.value });
//         }
//         else {
//             console.log(`No document found with ticker: ${companyTicker}`);
//             res.status(404).json({ message: `No document found with ticker: ${companyTicker}.` });
//         }
//     }
//     catch (error) {
//         console.error("Error deleting document:", error);
//         res.status(500).send("Failed to delete document.");
//     }
// });

const moment = require('moment');
const marketHolidays = [
    "2024-01-01", "2024-01-15", "2024-02-19", "2024-03-29",
    "2024-05-27", "2024-06-19", "2024-07-04", "2024-09-02",
    "2024-11-28", "2024-12-25", "2024-10-14", "2024-11-11"
];

function isMarketOpen(date) {

    if (date.day() === 6 || date.day() === 0) {
        return false;
    }
    if (marketHolidays.includes(date.format('YYYY-MM-DD'))) {
        return false;
    }
    return true;
}

function getPreviousMarketOpenDay(date) {
    while (!isMarketOpen(date)) {
        date = date.subtract(1, 'days');
    }
    return date;
}

function getMarketOpenDates() {
    let today = moment();
    let fromDate, toDate;
    if (isMarketOpen(today)) {
        fromDate = getPreviousMarketOpenDay(moment(today).subtract(1, 'days'));
        toDate = today;
    }
    else {
        toDate = getPreviousMarketOpenDay(today);
        fromDate = getPreviousMarketOpenDay(moment(toDate).subtract(1, 'days'));
    }
    return { fromDate: fromDate.format('YYYY-MM-DD'), toDate: toDate.format('YYYY-MM-DD') };
}

const { fromDate, toDate } = getMarketOpenDates();
console.log(`From Date: ${fromDate}, To Date: ${toDate}`);

const subDays = (days) => {
    return moment().subtract(days, 'days').format('YYYY-MM-DD');
};

const subMonths = (months, days) => {
    return moment().subtract(months, 'months').subtract(days, 'days').format('YYYY-MM-DD');
};

app.get('/search-company', async (req, res) => {
    const companyId = req.query.q;
    const token = process.env.FINNHUB_TOKEN;
    try {
        const response = await axios.get(`https://finnhub.io/api/v1/search?q=${companyId}&token=${token}`);
        res.json(response.data);
    }
    catch (error) {
        console.error('Error fetching search data:', error);
        res.status(500).send('Failed to fetch search data');
    }
});

app.get('/watch-company', async (req, res) => {
    const companyId = req.query.company_id;
    const token = process.env.FINNHUB_TOKEN;
    const quoteUrl = `https://finnhub.io/api/v1/quote?symbol=${companyId}&token=${token}`;
    const profileUrl = `https://finnhub.io/api/v1/stock/profile2?symbol=${companyId}&token=${token}`;

    try {
        const [quoteResponse, profileResponse] = await Promise.all([
            axios.get(quoteUrl),
            axios.get(profileUrl)
        ]);

        const combinedData = {
            ...quoteResponse.data,
            ...profileResponse.data,
            ticker: companyId
        };

        res.json(combinedData);
    }
    catch (error) {
        console.error('Error fetching company data:', error.message);
        res.status(500).send('Failed to fetch company data');
    }
});

app.get('/get-company', async (req, res) => {
    const companyId = req.query.company_id;
    const token = process.env.FINNHUB_TOKEN;
    const polygonKey = process.env.POLYGON_KEY;

    try {
        const profileResponse = await axios.get(`https://finnhub.io/api/v1/stock/profile2?symbol=${companyId}&token=${token}`);
        if (!profileResponse.data || Object.keys(profileResponse.data).length === 0) {
            return res.status(404).json({ message: 'No data found. Please enter a valid Ticker.' });
        }

        const dataRequests = [
            axios.get(`https://finnhub.io/api/v1/quote?symbol=${companyId}&token=${token}`),
            axios.get(`https://finnhub.io/api/v1/stock/recommendation?symbol=${companyId}&token=${token}`),
            axios.get(`https://finnhub.io/api/v1/company-news?symbol=${companyId}&from=${subDays(7)}&to=${toDate}&token=${token}`),
            axios.get(`https://finnhub.io/api/v1/stock/insider-sentiment?symbol=${companyId}&from=2022-01-01&token=${token}`),
            axios.get(`https://finnhub.io/api/v1/stock/peers?symbol=${companyId}&token=${token}`),
            axios.get(`https://finnhub.io/api/v1/stock/earnings?symbol=${companyId}&token=${token}`),
            axios.get(`https://api.polygon.io/v2/aggs/ticker/${companyId}/range/1/day/${subMonths(24, 1)}/${toDate}?adjusted=true&sort=asc&apiKey=${polygonKey}`),
            axios.get(`https://api.polygon.io/v2/aggs/ticker/${companyId}/range/1/hour/${fromDate}/${toDate}?adjusted=true&sort=asc&apiKey=${polygonKey}`)
        ];

        const [
            quoteResponse,
            trendResponse,
            newsResponse,
            insiderResponse,
            peerResponse,
            earningResponse,
            historyResponse,
            hourlyResponse
        ] = await Promise.all(dataRequests);

        // console.log('Profile Response:', profileResponse.data);
        // console.log('newsResponse:', newsResponse.data);
        // console.log('historyResponse:', historyResponse.data);
        // console.log('hourlyReponse:', hourlyResponse.data);

        const responseData = {
            companyData: {
                Ticker: profileResponse.data.ticker,
                CompanysName: profileResponse.data.name,
                ExchangeCode: profileResponse.data.exchange,
                IPOStartDate: profileResponse.data.ipo,
                Industry: profileResponse.data.finnhubIndustry,
                Webpage: profileResponse.data.weburl,
                Logo: profileResponse.data.logo
            },

            companyQuote: {
                LastPrice: quoteResponse.data.c,
                Change: quoteResponse.data.d,
                ChangePercentage: quoteResponse.data.dp,
                CurrentTimestamp: quoteResponse.data.t,
                HighPrice: quoteResponse.data.h,
                LowPrice: quoteResponse.data.l,
                OpenPrice: quoteResponse.data.o,
                PrevClose: quoteResponse.data.pc,
                TimeStamp: quoteResponse.data.t
            },

            recommendationTrend: Array.isArray(trendResponse.data) ? trendResponse.data.map(trendItem => ({
                Buy: trendItem.buy,
                Hold: trendItem.hold,
                period: trendItem.period,
                sell: trendItem.sell,
                strongBuy: trendItem.strongBuy,
                strongSell: trendItem.strongSell,
                symbol: trendItem.symbol,
            })) : [],

            companyEarnings: Array.isArray(earningResponse.data) ? earningResponse.data.map(earningItem => ({
                actual: earningItem.actual,
                estimate: earningItem.estimate,
                period: earningItem.period,
                symbol: earningItem.symbol,
                surprise: earningItem.surprise
            })) : [],

            companyInsider: insiderResponse.data && Array.isArray(insiderResponse.data.data) ? insiderResponse.data.data.map(insiderItem => ({
                symbol: insiderItem.symbol,
                change: insiderItem.change,
                mspr: insiderItem.mspr,
            })) : [],

            companyPeer: peerResponse.data || [],

            newsSet: Array.isArray(newsResponse.data) ? newsResponse.data.map(newsItem => ({
                Image: newsItem.image,
                Title: newsItem.headline,
                Description: newsItem.summary,
                LinkToOriginalPost: newsItem.url,
                Source: newsItem.source,
                PublishedDate: newsItem.datetime,
            })) : [],

            tradeSet: historyResponse.data.results ? historyResponse.data.results.map(trade => ({
                Date: trade.t,
                StockPrice: trade.c,
                Volume: trade.v,
                Open: trade.o,
                High: trade.h,
                Low: trade.l,
                Close: trade.c
            })) : [],

            hourlyTradeSet: hourlyResponse.data.results ? hourlyResponse.data.results.map(hourlyTrade => ({
                Date: hourlyTrade.t,
                StockPrice: hourlyTrade.c,
                Volume: hourlyTrade.v,
            })) : [],

            lastOpenDate: toDate,

        };

        res.json(responseData);
    }
    catch (error) {
        console.error('Error fetching profile data:', error.response ? error.response.data : error.message);
        res.status(500).json({ error: 'Failed to fetch company data', details: error.message });
    }

});

// app.get('*', (req, res) => {
//     res.sendFile(path.join(__dirname, '../build/index.html'));
// });

app.listen(PORT, () => {
    console.log(`Server listening on port ${PORT}...`);
});
