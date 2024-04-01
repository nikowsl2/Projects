import React from 'react';
import { BrowserRouter as Router, Routes, Route, NavLink, Navigate } from 'react-router-dom';
import MainPage from './components/MainPage/MainPage';
import WatchListPage from './components/WatchListPage/WatchListPage';
import { SearchProvider } from './components/SearchResult/SearchResult';
import PortfolioPage from './components/Portfolio/Portfolio';
import SearchPage from './components/SearchPage/SearchPage';
import './App.css';
import { useNavigate, useLocation } from 'react-router-dom';
import { useSearch } from './components/SearchResult/SearchResult';
import { Navbar, Nav } from 'react-bootstrap';
import 'bootstrap/dist/css/bootstrap.min.css';

const NavigationLinks = () => {
  const navigate = useNavigate();
  const { lastSearch } = useSearch();
  const location = useLocation();
  const checkActive = (path) => location.pathname.startsWith(path);
  const handleNavigate = (event) => {
    event.preventDefault();
    const destination = lastSearch ? `/search/${lastSearch}` : `/search/home`;
    navigate(destination);
  };

  return (
    <div className="link">
      <Navbar className='bg-body-tertiar' collapseOnSelect expand="lg">
        <Navbar.Brand className='brand' href="#home" style={{ color: 'white' }}>Stock Search</Navbar.Brand>
        <Navbar.Toggle aria-controls="responsive-navbar-nav" className="custom-toggler" />
        <Navbar.Collapse id="responsive-navbar-nav">
          <Nav className={`ml-auto`} >
            <NavLink to="/search/home" onClick={handleNavigate} className={checkActive("/search") ? "nav-link active-link" : "nav-link"}>
              Search
            </NavLink>
            <NavLink to="/watchlist" className={({ isActive }) => isActive ? "nav-link active-link" : "nav-link"}>
              Watchlist
            </NavLink>
            <NavLink to="/portfolio" className={({ isActive }) => isActive ? "nav-link active-link" : "nav-link"}>
              Portfolio
            </NavLink>
          </Nav>
        </Navbar.Collapse>
      </Navbar>
      <footer>
        <strong>Powered by</strong>&nbsp;<a href="https://finnhub.io/">Finnhub.io</a>
      </footer>
    </div >
  );
};

const App = () => {
  return (
    <SearchProvider>
      <Router>
        <NavigationLinks />
        <Routes>
          <Route path="/" element={<Navigate replace to="/search/home" />} />
          <Route path="/search/home" element={<MainPage />} />
          <Route path="/search/:ticker" element={<SearchPage />} />
          <Route path="/watchlist" element={<WatchListPage />} />
          <Route path="/portfolio" element={<PortfolioPage />} />
        </Routes>
      </Router>
    </SearchProvider>
  );
};

export default App;