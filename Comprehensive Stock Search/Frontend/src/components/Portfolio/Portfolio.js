import './Portfolio.css'
import React, { useEffect, useState } from 'react';
import Modal from '../Modal/Modal';
import { useNavigate } from 'react-router-dom';
import { Container, Row, Col } from 'react-bootstrap';

const fetchCompanyData = async (symbol) => {
    try {
        const response = await fetch(`https://wsl571-assignment3node-418610.wl.r.appspot.com/watch-company?company_id=${symbol}`);
        if (!response.ok) {
            const errorData = await response.json();
            throw new Error(errorData.message || `Error fetching data for ${symbol}: ${response.statusText}`);
        }
        const data = await response.json();

        return { ...data, ticker: symbol };
    } catch (error) {
        console.error('Error fetching company data:', error);
        throw error;
    }
};

const formatPrice = (price) => isNaN(price) ? 'N/A' : Number(price).toFixed(2);
const PortfolioPage = () => {
    const navigate = useNavigate();
    const handleHeaderClick = (tickerSymbol) => {
        navigate(`/search/${tickerSymbol}`);
    };
    const [AlertMessage, setAlertMessage] = useState('');
    const [refreshTrigger, setRefreshTrigger] = useState(0);
    const [aggregatedStocks, setAggregatedStocks] = useState([]);
    const [AlertType, setAlertType] = useState('');
    const [numberOfShares, setNumberOfShares] = useState('');
    const [balance, setBalance] = useState(0);
    const [tickers, setTickers] = useState([]);
    const [isBuyModalOpen, setIsBuyModalOpen] = useState(false);
    const [isSellModalOpen, setIsSellModalOpen] = useState(false);
    const [errorMsg, setErrorMsg] = useState('');
    const [errorMsg2, setErrorMsg2] = useState('');
    const [selectedStock, setSelectedStock] = useState(null);
    const [isBuyDisabled, setIsBuyDisabled] = useState(false);
    const [isSellDisabled, setIsSellDisabled] = useState(false);
    const [isLoading, setIsLoading] = useState(false);

    useEffect(() => {
        let timeoutId;                                                                  //from ChatGPT
        if (AlertMessage && AlertType) {
            timeoutId = setTimeout(() => {                                              //from ChatGPT
                setAlertMessage('');
                setAlertType('');
            }, 5000);
        }
        return () => clearTimeout(timeoutId);                                           //from ChatGPT
    }, [AlertMessage, AlertType]);

    useEffect(() => {
        const fetchTickers = async () => {
            setIsLoading(true);
            try {
                const response = await fetch('https://wsl571-assignment3node-418610.wl.r.appspot.com/api/portfolio/tickers');
                if (!response.ok) {
                    throw new Error('Failed to fetch tickers');
                }
                const data = await response.json();
                setTickers(data);
            } catch (error) {
                console.error("Error fetching tickers:", error);
            }
            setIsLoading(false);
        };

        fetchTickers();
    }, [refreshTrigger]);

    useEffect(() => {
        const fetchAndAggregateData = async () => {
            const aggregatedData = await Promise.all(tickers.map(async (tickerObj) => {
                const ticker = tickerObj._id;
                try {

                    const portfolioResponse = await fetch(`https://wsl571-assignment3node-418610.wl.r.appspot.com/api/portfolio/search?ticker=${ticker}`);
                    if (!portfolioResponse.ok) {
                        throw new Error(`Failed to fetch portfolio data for ${ticker}`);
                    }
                    const portfolioData = await portfolioResponse.json();

                    const companyData = await fetchCompanyData(ticker);

                    let totalShares = 0;
                    let totalCost = 0;

                    portfolioData.forEach(stock => {
                        totalShares += stock.shares;
                        totalCost += stock.shares * stock.purchasePrice;
                    });

                    return {
                        ...companyData,
                        totalShares,
                        averageCost: totalCost / totalShares,
                        totalCost,
                    };
                } catch (error) {
                    console.error(`Error fetching or aggregating data for ${ticker}:`, error);
                    return null; // Handle errors more gracefully as needed
                }
            }));

            setAggregatedStocks(aggregatedData.filter(stock => stock !== null));
        };

        if (tickers.length > 0) {
            fetchAndAggregateData();
        }
        else {
            console.log("current Portfolio list", tickers);
            setAggregatedStocks([]);
        }
    }, [tickers]);

    useEffect(() => {
        const checkBalance = async () => {
            try {
                const response = await fetch('https://wsl571-assignment3node-418610.wl.r.appspot.com/api/balance/search');
                if (!response.ok) {

                    throw new Error('Failed to fetch balance. Status: ' + response.status);
                }
                const data = await response.json();

                setBalance(data.balance);
            }
            catch (error) {
                console.error("Error fetching balance:", error);
            }
        };
        checkBalance();
    }, []);

    const openBuyModal = (stock) => {
        setSelectedStock(stock);
        setErrorMsg('');
        setIsBuyModalOpen(true);
        setNumberOfShares('');
    };

    const openSellModal = (stock) => {
        setSelectedStock(stock);
        setErrorMsg2('');
        setIsSellModalOpen(true);
        setNumberOfShares('');
    };

    useEffect(() => {
        if (selectedStock && numberOfShares) {
            const totalCost = parseFloat(numberOfShares) * selectedStock.c;
            if (totalCost > balance) {
                setErrorMsg('Not enough money in wallet!');
            } else {
                setErrorMsg('');
            }
        }
    }, [numberOfShares, selectedStock, balance]);

    useEffect(() => {
        if (selectedStock && numberOfShares) {
            const isSellDisabled = numberOfShares > selectedStock.totalShares || numberOfShares <= 0;
            if (isSellDisabled) {
                setErrorMsg2('Not enough shares!');
            } else {
                setErrorMsg2('');
            }
        }
    }, [numberOfShares, balance]);

    const BuyStock = async (event) => {
        event.preventDefault();
        const totalCost = numberOfShares * selectedStock.c;
        const newBalance = balance - totalCost;
        try {

            const requestBody = JSON.stringify({
                companyTicker: selectedStock.ticker,
                companyName: selectedStock.name,
                numberOfShares: numberOfShares,
                sharePrice: selectedStock.c,
            });

            const response = await fetch(`https://wsl571-assignment3node-418610.wl.r.appspot.com/api/portfolio/purchase`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: requestBody,
            });

            if (!response.ok) {
                throw new Error('Network response was not ok');
            }

            const balanceResponse = await fetch('https://wsl571-assignment3node-418610.wl.r.appspot.com/api/balance/update', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ balance: newBalance }),
            });

            if (!balanceResponse.ok) {
                throw new Error('Failed to update balance');
            }

            setBalance(newBalance);
            console.log("Stock purchased and balance updated successfully");
            const result = await response.json();
            console.log("Stock purchased successfully:", result);
            setIsBuyModalOpen(false);
            setNumberOfShares('');
            setAlertMessage(`${selectedStock.ticker} bought successfully.`)
            setAlertType('buy');
            setRefreshTrigger(trigger => trigger + 1);
        } catch (error) {
            console.error("Failed to buy stock:", error);
            setIsBuyModalOpen(false);
            setNumberOfShares('');
            setAlertMessage(`Failed to buy ${selectedStock.ticker}.`);
            setAlertType('sell');
        }
    };

    const SellStock = async (event) => {
        event.preventDefault();

        const totalSaleValue = numberOfShares * selectedStock.c;
        const newBalance = balance + totalSaleValue;

        try {
            const sellRequestBody = JSON.stringify({
                ticker: selectedStock.ticker,
                sellPrice: selectedStock.c,
                numberOfSharesToSell: numberOfShares,
            });

            const sellResponse = await fetch(`https://wsl571-assignment3node-418610.wl.r.appspot.com/api/portfolio/sell`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: sellRequestBody,
            });

            if (!sellResponse.ok) {
                throw new Error('Failed to sell stock');
            }

            const balanceResponse = await fetch('https://wsl571-assignment3node-418610.wl.r.appspot.com/api/balance/update', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ balance: newBalance }),
            });

            if (!balanceResponse.ok) {
                throw new Error('Failed to update balance');
            }

            setBalance(newBalance);
            console.log("Stock sold and balance updated successfully");

            setIsSellModalOpen(false);
            setNumberOfShares('');
            setAlertMessage(`${selectedStock.ticker} sold successfully.`)
            setAlertType('sell');
            setRefreshTrigger(trigger => trigger + 1);
        } catch (error) {
            console.error("Failed to sell stock:", error);
            setIsSellModalOpen(false);
            setNumberOfShares('');
            setAlertMessage(`Failed to sell ${selectedStock.ticker}.`);
            setAlertType('error');
        }
    };

    const CloseAlert = (event) => {
        event.preventDefault();
        setAlertMessage('');
        setAlertType('');
    }

    const calculateStyleAndContent = (currentValue, referenceValue) => {
        if (!currentValue) {
            return { color: 'black', content: 'N/A' };
        }
        const difference = currentValue - referenceValue;
        const isDifferenceNegligible = Math.abs(difference) < 0.01;
        const formattedDifference = formatPrice(difference);
        const color = !isDifferenceNegligible ? (difference > 0 ? 'green' : 'red') : 'black';
        const content = isDifferenceNegligible ? '0.00' : formattedDifference;
        const symbol = !isDifferenceNegligible ? (difference > 0 ? '▲' : '▼') : null;
        return { color, content, symbol };
    };

    const alertClass = AlertType === 'buy' ? 'ModalAlertMessage ModalAlertBuy' : 'ModalAlertMessage ModalAlertSell';
    useEffect(() => {
        if (selectedStock) {
            const totalCost = parseFloat(numberOfShares) * selectedStock.c;
            setIsBuyDisabled(isNaN(totalCost) || totalCost > balance || numberOfShares <= 0);
            setIsSellDisabled(numberOfShares > selectedStock.totalShares || numberOfShares <= 0);
        } else {
            setIsBuyDisabled(true);
            setIsSellDisabled(true);
        }
    }, [selectedStock, numberOfShares, balance]);

    if (isLoading) {
        return <div className="loader2"></div>;
    }

    return (
        <div className='PortfolioBox'>
            {AlertMessage && <div className={alertClass}>{AlertMessage}
                <button className='ModalAlertButton' onClick={CloseAlert} >
                    X
                </button>
            </div>}
            <div className='PortfolioTitle'>My Portfolio</div>
            <div className='Wallet'>Money in Wallet: ${formatPrice(balance)}</div>

            <Container fluid>
                {aggregatedStocks.length > 0 ? aggregatedStocks.map((stock, index) => {
                    const { color: changeColor, content: changeContent, symbol: changeSymbol } = calculateStyleAndContent(stock.c, stock.totalCost / stock.totalShares);
                    return (
                        <div key={index} className="stock-table custom-row-padding">
                            <Row onClick={() => handleHeaderClick(stock.ticker)} style={{ cursor: 'pointer', marginBottom: '10px' }}>
                                <Col className='PortfolioIntro'>
                                    <div >
                                        <span style={{ fontSize: '20px', fontWeight: '500' }}>{stock.ticker}</span>
                                        <span style={{ marginLeft: '8px', color: 'grey' }}>{stock.name || 'N/A'}</span>
                                    </div>
                                </Col>
                            </Row>
                            <Row className="row-gap">
                                <Col md={6} className="d-flex flex-column justify-content-between">
                                    <div className='TableRow'>
                                        <span>Quantity:</span><span>{stock.totalShares}</span>
                                    </div>
                                    <div className='TableRow'>
                                        <span>Avg. Cost / Share:</span><span>{formatPrice(stock.totalCost / stock.totalShares)}</span>
                                    </div>
                                    <div className='TableRow'>
                                        <span>Total Cost:</span><span>{formatPrice(stock.totalCost)}</span>
                                    </div>
                                </Col>
                                <Col md={6} className="d-flex flex-column justify-content-between" style={{ color: changeColor }}>
                                    <div className='TableRow'>
                                        <span>Change:</span><span>{changeSymbol}{changeContent}</span>
                                    </div>
                                    <div className='TableRow'>
                                        <span>Current Price:</span><span>{stock.c ? formatPrice(stock.c) : 'N/A'}</span>
                                    </div>
                                    <div className='TableRow'>
                                        <span>Market Value:</span><span>{formatPrice(stock.c * stock.totalShares)}</span>
                                    </div>
                                </Col>
                            </Row>
                            <div className='PortfolioTradeButton'>
                                <button className='PortfolioBuyStock' onClick={() => openBuyModal(stock)}>Buy</button>
                                <button className='PortfolioSellStock' onClick={() => openSellModal(stock)}>Sell</button>
                            </div>
                        </div>
                    );
                }) : (
                    <div className="alert-message">Currently you don't have any stock.</div>
                )}
            </Container>
            {selectedStock && (
                <React.Fragment>
                    <Modal isOpen={isBuyModalOpen} closeModal={() => setIsBuyModalOpen(false)}>
                        <div>
                            <p className='BuyTitle'>{selectedStock?.ticker}</p>
                            <div className="Buy-divider"></div>
                            <p className='CurrentPrice'>Current Price: {formatPrice(selectedStock?.c)}</p>
                            <p className='Balance'>Money in Wallet: ${formatPrice(balance)}</p>
                            <form className='BuyForm' onSubmit={BuyStock}>
                                Quantity:
                                <input
                                    type="number"
                                    value={numberOfShares}
                                    className='BuyBox'
                                    onChange={(e) => setNumberOfShares(e.target.value)}
                                    placeholder="Number of Shares"
                                    min="1" />
                                {errorMsg && <p className="errorMsg">{errorMsg}</p>}
                                <div className="Buy-divider"></div>
                                <p className='TotalCost'>Total: {formatPrice(numberOfShares * selectedStock?.c)}</p>
                                <button className={`ModalBuyStock ${isBuyDisabled ? 'ModalBuyStockDisabled' : 'ModalBuyStock'}`} onClick={BuyStock} disabled={isBuyDisabled}>
                                    Buy
                                </button>
                            </form>
                        </div>
                    </Modal>
                    <Modal isOpen={isSellModalOpen} closeModal={() => setIsSellModalOpen(false)}>
                        <div>
                            <p className='BuyTitle'>{selectedStock?.ticker}</p>
                            <div className="Buy-divider"></div>
                            <p className='CurrentPrice'>Current Price: {formatPrice(selectedStock?.c)}</p>
                            <p className='Balance'>Shares held: {selectedStock.totalShares}</p>
                            <form className='BuyForm' onSubmit={SellStock}>
                                Quantity:
                                <input
                                    type="number"
                                    value={numberOfShares}
                                    className='BuyBox'
                                    onChange={(e) => setNumberOfShares(e.target.value)}
                                    placeholder="Number of Shares"
                                    min="1" />
                                {errorMsg2 && <p className="errorMsg2">{errorMsg2}</p>}
                                <div className="Buy-divider"></div>
                                <p className='TotalCost'>Total: {formatPrice(numberOfShares * selectedStock?.c)}</p>
                                <button className={`ModalSellStock ${isSellDisabled ? 'ModalSellStockDisabled' : 'ModalSellStock'}`} onClick={SellStock} disabled={isSellDisabled}>
                                    Sell
                                </button>
                            </form>
                        </div>
                    </Modal>
                </React.Fragment>
            )}
        </div>
    );
};



export default PortfolioPage;


