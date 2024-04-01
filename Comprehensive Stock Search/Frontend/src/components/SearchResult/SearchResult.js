// SearchContext.js
import React, { createContext, useContext, useState } from 'react';

const SearchContext = createContext();

export function useSearch() {
    return useContext(SearchContext);
}

export const SearchProvider = ({ children }) => {
    const [searchResults, setSearchResults] = useState(null);
    const [lastSearch, setLastSearch] = useState('');

    return (
        <SearchContext.Provider value={{ searchResults, setSearchResults, lastSearch, setLastSearch }}>
            {children}
        </SearchContext.Provider>
    );
};
