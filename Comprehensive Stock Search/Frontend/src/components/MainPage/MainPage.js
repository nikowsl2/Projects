import React from 'react';
import MainPageAutocomplete from '../Autocomplete/MainPageAutocomplete';
import { useNavigate } from 'react-router-dom';

const MainPage = () => {
    const navigate = useNavigate();
    const directToSearch = (data) => {
        if (data) {
            navigate(`/search/${data}`);
        }
    };

    return (
        <div className="container">
            <div className="searchBox">
                <p className="searchTitle">STOCK SEARCH</p>
                <MainPageAutocomplete onFetchData={directToSearch} />
            </div>
        </div>
    );
};

export default MainPage;