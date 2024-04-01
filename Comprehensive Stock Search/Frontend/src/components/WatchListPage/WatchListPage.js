import React, { useEffect, useState } from 'react';
import { getMarketStatusMessage } from '../MarketStatus/MarketStatus';
import './WatchListPage.css';
import { useNavigate } from 'react-router-dom';

const fetchCompanyData = async (symbol) => {
    try {
        const response = await fetch(`https://wsl571-assignment3node-418610.wl.r.appspot.com/watch-company?company_id=${symbol}`);
        if (!response.ok) {
            const errorData = await response.json();
            throw new Error(errorData.message || `Error fetching data for ${symbol}: ${response.statusText}`);
        }
        const data = await response.json();

        return { ...data, ticker: symbol };
    }
    catch (error) {
        console.error('Error fetching company data:', error);
        throw error;
    }
};

const fetchWatchlist = async () => {
    try {
        const response = await fetch('https://wsl571-assignment3node-418610.wl.r.appspot.com/api/watchlist/getList');
        if (!response.ok) {
            throw new Error(`Failed to fetch watchlist: ${response.statusText}`);
        }
        const data = await response.json();
        return data;
    }
    catch (error) {
        console.error('Error fetching watchlist:', error);
        throw error;
    }
};

const formatPrice = (price) => isNaN(price) ? 'N/A' : Number(price).toFixed(2);
const WatchListPage = () => {
    const navigate = useNavigate();
    const [watchlist, setWatchlist] = useState([]);
    const handleHeaderClick = (tickerSymbol) => {
        navigate(`/search/${tickerSymbol}`);
    };
    const [updatedData, setUpdatedData] = useState([]);
    const [errors, setErrors] = useState([]);
    const [isLoading, setIsLoading] = useState(false);

    useEffect(() => {
        const initFetch = async () => {
            setIsLoading(true);
            try {
                const watchlistData = await fetchWatchlist();
                setWatchlist(watchlistData);
                await fetchAllCompanyData();
                setIsLoading(false);
            }
            catch (error) {
                setErrors(prev => [...prev, error.message]);
                setIsLoading(false);
            }
        };

        initFetch();
    }, []);

    const handleRemoveItem = async (ticker) => {
        try {
            await fetch(`https://wsl571-assignment3node-418610.wl.r.appspot.com/api/watchlist/delete?ticker=${ticker}`,
                {
                    method: 'DELETE'
                });

            setWatchlist(currentWatchlist => currentWatchlist.filter(item => item.ticker !== ticker));
        }
        catch (error) {
            console.error(`Error removing ${ticker} from watchlist:`, error);
            setErrors(prev => [...prev, error.message]);
        }
    };

    const fetchAllCompanyData = async () => {
        const promises = watchlist.map(company =>
            fetchCompanyData(company.ticker).catch(error => setErrors(prev => [...prev, error.message]))
        );
        const results = await Promise.all(promises);
        setUpdatedData(results.filter(data => data !== undefined));
    };

    useEffect(() => {
        fetchAllCompanyData();
        const marketStatus = getMarketStatusMessage();
        let interval;
        if (marketStatus.includes("Open")) {
            interval = setInterval(fetchAllCompanyData, 15000);
        }
        return () => {
            if (interval) {
                clearInterval(interval);
            }
        };
    }, [watchlist]);

    if (isLoading) {
        return <div className="loader2"></div>;
    }

    return (
        <div className='WatchPageBox'>
            <div className='WatchPageTitle'>My WatchList</div>
            {errors.length > 0 && (
                <ul>
                    {errors.map((error, index) => <li key={index}>{error}</li>)}
                </ul>
            )}
            {watchlist.length === 0 ? (<div className="alert-message">Currently you don't have any stock in your watchlist.</div>) : (
                <div className="watchlist-tabs">
                    {updatedData.map((company, index) => {
                        const companyInfo = watchlist.find(item => item.ticker === company.ticker);
                        return (
                            <div key={index} className="watchlist-tab" onClick={() => handleHeaderClick(company.ticker)} >
                                <div className="company-info">
                                    <div className="ticker">{companyInfo ? `${company.ticker}` : 'N/A'}</div>
                                    <div className="company-name">{companyInfo ? `${company.name}` : ''}</div>
                                </div>
                                <div className="price-info">
                                    <div className="last-price" style={{ color: company.d > 0 ? 'green' : company.d < 0 ? 'red' : 'black' }}>
                                        {formatPrice(company.c)}
                                    </div>
                                    <div className="price-change" style={{ color: company.d > 0 ? 'green' : company.d < 0 ? 'red' : 'black' }}>
                                        {company.d > 0 ? '▲' : company.d < 0 ? '▼' : null}
                                        {formatPrice(company.d)} ({formatPrice(company.dp)}%)
                                    </div>
                                </div>
                                <button onClick={(e) => {
                                    e.stopPropagation();                                       //from ChatGPT
                                    handleRemoveItem(company.ticker);
                                }} className="remove-tab" style={{ cursor: 'pointer' }} >X</button>
                            </div>
                        );
                    })}
                </div>
            )}
        </div>
    );
};

export default WatchListPage;


