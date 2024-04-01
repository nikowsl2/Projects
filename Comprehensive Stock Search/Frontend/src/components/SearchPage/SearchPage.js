import React, { useState, useEffect } from 'react';
import Autocomplete from '..//Autocomplete/Autocomplete';
import Infotable from '..//Infotable/Infotable';
import SummaryNav from '..//Companysummary/Companysummary';
import DetailInfo from '..//DisplayInfo/DisplayInfo';
import NewsInfo from '..//NewsInfo/NewsInfo';
import ChartInfo from '..//ChartInfo/ChartInfo';
import InsightInfo from '..//InsightInfo/InsightInfo';
import { useSearch } from '../SearchResult/SearchResult';
import { useParams, useNavigate } from 'react-router-dom';
import { useLocation } from 'react-router-dom';

const SearchPage = () => {
    const [companyData, setCompanyData] = useState(null);
    const [selectedView, setSelectedView] = useState('summary');
    const { searchResults, setSearchResults, setLastSearch } = useSearch();

    const { ticker } = useParams();
    const dataToDisplay = searchResults || companyData;
    const location = useLocation();

    useEffect(() => {
        // console.log(ticker)
        if (ticker) {
            if (searchResults && ticker === searchResults.symbol) {
                setCompanyData(searchResults);
            }
        }
    }, [ticker, searchResults]);

    useEffect(() => {
        if (location.state && location.state.symbol) {                    //from ChatGPT
            handleSubmit(location.state.symbol);                          //from ChatGPT
        }
    }, [location]);


    const handleCompanyData = (data) => {
        console.log('getting onfetchData!', data);
        if (data && data.companyData.Ticker) {
            const existingData = searchResults && searchResults.companyData.Ticker === data.companyData.Ticker;
            if (!existingData) {
                setCompanyData(data);
                setSearchResults(data);
                setLastSearch(data.companyData.Ticker);
                console.log('NEW DATA!');
                navigate(`/search/${data.companyData.Ticker}`);
            }
        }
        console.log('current Data!', dataToDisplay);
    };

    const navigate = useNavigate();
    const handleSubmit = (symbol) => {
        navigate(`/search/${symbol}`);
    };

    const changeView = (view) => {
        setSelectedView(view);
    };

    const clearContent = () => {
        setCompanyData(null);
        setSearchResults(null);
        setLastSearch('');
        setSelectedView('summary');
        navigate('/search/home');
    };

    return (
        <>
            <div className="container">
                <div className="searchBox">
                    <p className="searchTitle">STOCK SEARCH</p>
                    <Autocomplete symbol={ticker} onFetchData={handleCompanyData} onClear={clearContent} />
                </div>
            </div>
            <div className="infotableBox">
                {dataToDisplay && <Infotable companyData={dataToDisplay} />}
            </div>
            {dataToDisplay && (
                <div className="detailContainer">
                    <SummaryNav selectedView={selectedView} onChangeView={changeView} />
                </div>
            )}
            {selectedView === 'summary' && dataToDisplay && <DetailInfo handleSubmit={handleSubmit} companyData={dataToDisplay} />}
            {selectedView === 'news' && dataToDisplay && <NewsInfo companyData={dataToDisplay} />}
            {selectedView === 'charts' && dataToDisplay && <ChartInfo companyData={dataToDisplay} />}
            {selectedView === 'insights' && dataToDisplay && <InsightInfo companyData={dataToDisplay} />}
        </>
    );
};

export default SearchPage;

