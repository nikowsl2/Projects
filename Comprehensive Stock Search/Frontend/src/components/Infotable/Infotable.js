import React, { useEffect, useState } from 'react';
import './Infotable.css';
import { getMarketStatusMessage } from '../MarketStatus/MarketStatus';
import Modal from '../Modal/Modal';
import moment from 'moment-timezone';

const Infotable = ({ companyData }) => {
    const [numberOfShares, setNumberOfShares] = useState('');
    const [isBuyModalOpen, setIsBuyModalOpen] = useState(false);
    const [isSellModalOpen, setIsSellModalOpen] = useState(false);
    const { Ticker, CompanysName, Logo, ExchangeCode } = companyData.companyData;
    const [currentPSTTime, setCurrentPSTTime] = useState(moment().tz('America/Los_Angeles').format('YYYY-MM-DD HH:mm:ss'));
    const LastOpened = companyData.lastOpenDate;
    const marketStatusMessage = getMarketStatusMessage(LastOpened);
    const [errorMsg, setErrorMsg] = useState('');
    const [errorMsg2, setErrorMsg2] = useState('');
    const [AlertMessage, setAlertMessage] = useState('');
    const [WatchListMessage, setWatchListMessage] = useState('');
    const [NotinWatchListMessage, setNotinWatchListMessage] = useState('');
    const [AlertType, setAlertType] = useState('');
    const [refreshTrigger, setRefreshTrigger] = useState(0);
    const [aggregatedData, setAggregatedData] = useState({ totalShares: 0, averageCost: 0 });
    const [companyQuote, setCompanyQuote] = useState({
        LastPrice: companyData.companyQuote?.LastPrice ?? 0,
        Change: companyData.companyQuote?.Change ?? 0,
        ChangePercentage: companyData.companyQuote?.ChangePercentage ?? 0,
        CurrentTimestamp: companyData.companyQuote?.CurrentTimestamp ?? 0,
        TimeStamp: companyData.companyQuote?.Timestamp ?? 0
    });
    const [isInWatchlist, setIsInWatchlist] = useState(false);
    const [balance, setBalance] = useState(0);

    useEffect(() => {
        console.log(`Current ticker: ${Ticker}`);
    }, [Ticker]);

    useEffect(() => {
        setCompanyQuote({
            LastPrice: companyData.companyQuote?.LastPrice ?? 0,
            Change: companyData.companyQuote?.Change ?? 0,
            ChangePercentage: companyData.companyQuote?.ChangePercentage ?? 0,
            CurrentTimestamp: companyData.companyQuote?.CurrentTimestamp ?? 0,
            TimeStamp: companyData.companyQuote?.Timestamp ?? 0
        });
    }, [companyData]);

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

    useEffect(() => {
        const checkIfInWatchlist = async () => {
            try {
                const response = await fetch(`https://wsl571-assignment3node-418610.wl.r.appspot.com/api/watchlist/search?ticker=${Ticker}`);
                console.log('search this ticker in watchlist', Ticker);
                const data = await response.json();
                setIsInWatchlist(data.found);

            }
            catch (error) {
                console.error("Error checking watchlist status:", error);
            }
        };

        checkIfInWatchlist();
    }, [Ticker]);

    useEffect(() => {
        console.log('currentIsinWatchList!', isInWatchlist);
    }, [isInWatchlist]);

    const toggleWatchlistItem = async () => {
        try {
            let method, endpoint, options;
            if (isInWatchlist) {
                method = 'DELETE';
                endpoint = `https://wsl571-assignment3node-418610.wl.r.appspot.com/api/watchlist/delete?ticker=${Ticker}`;
                options = {
                    method: method,
                    headers: {
                        'Content-Type': 'application/json',
                    },
                };
            }
            else {
                method = 'POST';
                endpoint = `https://wsl571-assignment3node-418610.wl.r.appspot.com/api/watchlist/insert`;
                options = {
                    method: method,
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({ ticker: Ticker }),
                };
            }

            await fetch(endpoint, options);
            // try {
            //     let method, endpoint;
            //     if (isInWatchlist) {
            //         method = 'DELETE';
            //         endpoint = `https://wsl571-assignment3node-418610.wl.r.appspot.com/api/watchlist/delete/${Ticker}`;
            //     }
            //     // if (isInWatchlist) {
            //     //     method = 'DELETE';
            //     //     endpoint = `https://wsl571-assignment3node-418610.wl.r.appspot.com/api/watchlist/delete`;
            //     // }
            //     else {
            //         method = 'POST';
            //         endpoint = `https://wsl571-assignment3node-418610.wl.r.appspot.com/api/watchlist/insert`;
            //     }

            //     await fetch(endpoint, {
            //         method: method,
            //         headers: {
            //             'Content-Type': 'application/json',
            //         },
            //         body: JSON.stringify({ ticker: Ticker }),
            //     });

            setIsInWatchlist(!isInWatchlist);

        } catch (error) {
            console.error("Failed to toggle watchlist item:", error);
        }
    };

    useEffect(() => {
        const fetchStockData = async () => {
            try {

                const response = await fetch(`https://wsl571-assignment3node-418610.wl.r.appspot.com/api/portfolio/search?ticker=${Ticker}`);
                if (response.status === 404) {
                    console.log(`No documents found for ticker ${Ticker}`);
                    setAggregatedData({ totalShares: 0, averageCost: 0 });
                    return;
                }

                if (!response.ok) {
                    throw new Error(`Failed to fetch data. Status: ${response.status}`);
                }

                const stocks = await response.json();
                let totalShares = 0;
                let totalCost = 0;

                stocks.forEach((stock) => {
                    totalShares += stock.shares;
                    totalCost += stock.cumulativeCost;
                });

                const averageCost = totalShares > 0 ? totalCost / totalShares : 0;
                setAggregatedData({ totalShares, averageCost });

            }
            catch (error) {
                console.error("Error calculating aggregated data:", error);
            }
        };

        if (Ticker) {
            fetchStockData();
        }
    }, [Ticker, refreshTrigger]);

    useEffect(() => {
        const fetchUpdatedData = async (symbol) => {
            try {
                const response = await fetch(`https://wsl571-assignment3node-418610.wl.r.appspot.com/watch-company?company_id=${symbol}`);
                if (!response.ok) {
                    const errorData = await response.json();
                    throw new Error(errorData.message || `Error fetching data for ${symbol}: ${response.statusText}`);
                }
                const quoteResponse = await response.json();
                console.log('updatequote', quoteResponse)

                setCompanyQuote({
                    LastPrice: quoteResponse.c,
                    Change: quoteResponse.d,
                    ChangePercentage: quoteResponse.dp,
                    CurrentTimestamp: quoteResponse.t,
                    TimeStamp: quoteResponse.t
                });
            } catch (error) {
                console.error('Error fetching company data:', error);
            }
        };
        let interval;
        console.log(marketStatusMessage);
        if (marketStatusMessage.includes('Open')) {
            fetchUpdatedData(Ticker);
            setCurrentPSTTime(moment().tz('America/Los_Angeles').format('YYYY-MM-DD HH:mm:ss'));
            interval = setInterval(() => {
                fetchUpdatedData(Ticker);
                setCurrentPSTTime(moment().tz('America/Los_Angeles').format('YYYY-MM-DD HH:mm:ss'));
            }, 15000);
        }

        return () => {
            if (interval) {
                clearInterval(interval);
            }
        };

    }, [Ticker, marketStatusMessage]);
    const { LastPrice, Change, ChangePercentage, CurrentTimestamp } = companyQuote;

    const BuyStock = async (event) => {
        event.preventDefault();
        const totalCost = numberOfShares * companyQuote.LastPrice;
        const newBalance = balance - totalCost;

        try {
            const requestBody = JSON.stringify({
                companyTicker: Ticker,
                companyName: CompanysName,
                numberOfShares: numberOfShares,
                sharePrice: companyQuote.LastPrice,
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
            setAlertMessage(`${Ticker} bought successfully.`);
            setAlertType('buy');
            setRefreshTrigger(trigger => trigger + 1);
            setTimeout(() => {
                setAlertMessage('');
                setAlertType('');
            }, 5000);
        }
        catch (error) {
            console.error("Failed to buy stock:", error);
            setIsBuyModalOpen(false);
            setNumberOfShares('');
            setAlertMessage(`Failed to buy ${Ticker}.`);
            setAlertType('sell');
        }
    };

    const SellStock = async (event) => {
        event.preventDefault();
        const totalSaleValue = numberOfShares * companyQuote.LastPrice;
        const newBalance = balance + totalSaleValue;

        try {
            const sellRequestBody = JSON.stringify({
                ticker: Ticker,
                sellPrice: companyQuote.LastPrice,
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
            setAlertMessage(`${Ticker} sold successfully.`);
            setAlertType('sell');
            setTimeout(() => {
                setAlertMessage('');
                setAlertType('');
            }, 5000);
            setRefreshTrigger(trigger => trigger + 1);
        }
        catch (error) {
            console.error("Failed to sell stock:", error);
            setIsSellModalOpen(false);
            setNumberOfShares('');
            setAlertMessage(`Failed to sell ${Ticker}.`);
            setAlertType('error');
        }
    };

    const formatNumber = (number) => {
        return isNaN(number) ? 'N/A' : Number(number).toFixed(2);
    };

    const openBuyModal = () => {
        setErrorMsg('');
        setIsBuyModalOpen(true);
        setNumberOfShares('');
    };

    const openSellModal = () => {
        setErrorMsg2('');
        setIsSellModalOpen(true);
        setNumberOfShares('');
    };

    useEffect(() => {
        const totalCost = numberOfShares * LastPrice;
        if (totalCost > balance) {
            setErrorMsg('Not enough money in wallet!');
        } else {
            setErrorMsg('');
        }
    }, [numberOfShares, balance, LastPrice]);

    useEffect(() => {
        const isStockEnough = numberOfShares > sharesOwned;
        if (isStockEnough) {
            setErrorMsg2('Not enough shares!');
        } else {
            setErrorMsg2('');
        }
    }, [numberOfShares, balance, LastPrice]);

    const CloseAlert = (event) => {
        event.preventDefault();
        setAlertMessage('');
        setAlertType('');
    }
    const CloseWatchListAlert = (event) => {
        event.preventDefault();
        setWatchListMessage('');
    }

    const handleWatchlistMessage = () => {
        const message = isInWatchlist
            ? `${Ticker} removed from watchlist.`
            : `${Ticker} added to watchlist.`;
        if (isInWatchlist) {
            setNotinWatchListMessage(message);
        }
        else {
            setWatchListMessage(message);
        }

        setTimeout(() => {
            setWatchListMessage('');
            setNotinWatchListMessage('');
        }, 5000);
    };

    const handleWatchList = () => {
        handleWatchlistMessage();
        toggleWatchlistItem({
            ticker: Ticker,
            name: CompanysName,
        })
    };

    const alertClass = AlertType === 'buy' ? 'AlertMessage AlertBuy' : 'AlertMessage AlertSell';
    const totalCost = LastPrice * numberOfShares;
    const isBuyDisabled = totalCost > balance || numberOfShares <= 0;
    const sharesOwned = aggregatedData.totalShares;
    const isSellDisabled = numberOfShares > sharesOwned || numberOfShares <= 0;


    return (
        <div>
            <div className='messageContainer'>
                {WatchListMessage && <div className={'watchlist-message in-watchlist'}>{WatchListMessage}
                    <button className='WatchListButton' onClick={CloseWatchListAlert}>
                        X
                    </button>
                </div>}
                {NotinWatchListMessage && <div className={'watchlist-message'}>{NotinWatchListMessage}
                    <button className='WatchListButton' onClick={CloseWatchListAlert}>
                        X
                    </button>
                </div>}
                {AlertMessage && <div className={alertClass}>{AlertMessage}
                    <button className='AlertButton' onClick={CloseAlert} >
                        X
                    </button>
                </div>}
            </div>
            <div className="info-container">
                <div className="info-column" style={{ width: '39%' }}>
                    <div><span style={{ fontSize: '26px' }}>{Ticker}</span>
                        <button onClick={() => handleWatchList()}
                            className={`toggle-watchlist-button ${isInWatchlist ? 'in-watchlist' : ''}`}>{isInWatchlist ? '\u2605' : '\u2606'}</button>
                    </div>
                    <div><span style={{ color: '#696969', fontSize: '22px' }}>{CompanysName}</span></div>
                    <div><span style={{ color: '#696969' }}>{ExchangeCode}</span></div>
                    <div className='TradeButton'>
                        <button className='BuyStock' onClick={() => openBuyModal()}>Buy</button>
                        {sharesOwned > 0 && (
                            <button className='SellStock' onClick={() => openSellModal()}>Sell</button>
                        )}
                    </div>
                    <Modal isOpen={isBuyModalOpen} closeModal={() => setIsBuyModalOpen(false)}>
                        <div>
                            <p className='BuyTitle'>{Ticker}</p>
                            <div className="Buy-divider"></div>
                            <p className='CurrentPrice'>Current Price:{formatNumber(LastPrice)}</p>
                            <p className='Balance'>Money in Wallet: ${formatNumber(balance)}</p>
                            <form className='BuyForm' onSubmit={BuyStock}>Quantity:
                                <input
                                    type="number"
                                    value={numberOfShares}
                                    className='BuyBox'
                                    onChange={(e) => setNumberOfShares(e.target.value)}
                                    placeholder="Number of Shares"
                                    min="1" />
                                {errorMsg && <p className="errorMsg">{errorMsg}</p>}
                                <div className="Buy-divider"></div>
                                <p className='TotalCost'>Total: {formatNumber(totalCost)} </p>
                                <button
                                    type="submit"
                                    className={`ModalBuyStock ${isBuyDisabled ? 'ModalBuyStockDisabled' : ''}`}
                                    disabled={isBuyDisabled}>
                                    Buy
                                </button>
                            </form>
                        </div>
                    </Modal>
                    <Modal isOpen={isSellModalOpen} closeModal={() => setIsSellModalOpen(false)}>
                        <div>
                            <p className='BuyTitle'>{Ticker}</p>
                            <div className="Buy-divider"></div>
                            <p className='CurrentPrice'>Current Price:{formatNumber(LastPrice)}</p>
                            <p className='Balance'>Shares holding: {sharesOwned}</p>
                            <form className='BuyForm' onSubmit={SellStock}>Quantity: <input
                                type="number"
                                value={numberOfShares}
                                className='BuyBox'
                                onChange={(e) => setNumberOfShares(e.target.value)}
                                placeholder="Number of Shares"
                                min="1" />
                                {errorMsg2 && <p className="errorMsg2">{errorMsg2}</p>}
                                <div className="Buy-divider"></div>
                            </form>
                            <p className='TotalCost'>Total: {formatNumber(totalCost)} </p>
                            <button className={`ModalSellStock ${isSellDisabled ? 'ModalSellStockDisabled' : 'ModalSellStock'}`} onClick={SellStock} disabled={isSellDisabled}>
                                Sell
                            </button>
                        </div>
                    </Modal>
                </div>
                <div className="info-column info-center" style={{ width: '14%' }}>
                    <img src={Logo} className='infoLogo' alt="Company Logo" style={{ maxWidth: '80px', maxHeight: '80px' }} /><br></br>
                    <span style={{ color: marketStatusMessage.includes("Open") ? 'green' : 'red', fontWeight: 'bold' }}>
                        {marketStatusMessage}
                    </span>
                </div>
                <div className="info-column" style={{ width: '39%' }}>
                    <div><span style={{ color: Change > 0 ? 'green' : Change < 0 ? 'red' : 'black', fontSize: '28px' }}>{formatNumber(LastPrice)}</span></div>
                    <div>
                        <span style={{ color: Change > 0 ? 'green' : Change < 0 ? 'red' : 'black', fontSize: '18px' }}>
                            {Change > 0 ? '▲' : Change < 0 ? '▼' : null} {formatNumber(Change)} ({formatNumber(ChangePercentage)}%)
                        </span>
                    </div>
                    <div>{currentPSTTime}</div>
                </div>
            </div>
        </div>
    );
};

export default Infotable;




