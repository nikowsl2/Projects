import React, { useEffect, useState } from 'react';
import './DisplayInfo.css';
import Highcharts from 'highcharts';
import HighchartsReact from 'highcharts-react-official';
import { getMarketStatusMessage } from '../MarketStatus/MarketStatus';
import { Container, Row, Col } from 'react-bootstrap';

const DetailInfo = ({ companyData, handleSubmit }) => {
    const handleListItemClick = (peer) => {
        handleSubmit(peer);
    };
    const LastOpened = companyData.lastOpenDate;
    const marketStatusMessage = getMarketStatusMessage(LastOpened);
    const { Webpage, Ticker, IPOStartDate, Industry } = companyData.companyData;
    const companyPeer = companyData.companyPeer;
    const hourlyTradeData = companyData.hourlyTradeSet || [];
    const PST_OFFSET = 7 * 60 * 60 * 1000;
    const chartSeriesData = hourlyTradeData.map(item => [
        item.Date - PST_OFFSET,
        item.StockPrice
    ]);
    const [companyQuote, setCompanyQuote] = useState({
        LastPrice: companyData.companyQuote?.LastPrice ?? 0,
        Change: companyData.companyQuote?.Change ?? 0,
        ChangePercentage: companyData.companyQuote?.ChangePercentage ?? 0,
        CurrentTimestamp: companyData.companyQuote?.CurrentTimestamp ?? 0,
        HighPrice: companyData.companyQuote?.HighPrice ?? 0,
        LowPrice: companyData.companyQuote?.LowPrice ?? 0,
        OpenPrice: companyData.companyQuote?.OpenPrice ?? 0,
        PrevClose: companyData.companyQuote?.PrevClose ?? 0,
        TimeStamp: companyData.companyQuote?.TimeStamp ?? 0
    });

    useEffect(() => {
        setCompanyQuote({
            LastPrice: companyData.companyQuote?.LastPrice ?? 0,
            Change: companyData.companyQuote?.Change ?? 0,
            ChangePercentage: companyData.companyQuote?.ChangePercentage ?? 0,
            CurrentTimestamp: companyData.companyQuote?.CurrentTimestamp ?? 0,
            HighPrice: companyData.companyQuote?.HighPrice ?? 0,
            LowPrice: companyData.companyQuote?.LowPrice ?? 0,
            OpenPrice: companyData.companyQuote?.OpenPrice ?? 0,
            PrevClose: companyData.companyQuote?.PrevClose ?? 0,
            TimeStamp: companyData.companyQuote?.TimeStamp ?? 0
        });
    }, [companyData]);

    useEffect(() => {
        const fetchUpdatedData = async (symbol) => {
            try {
                const response = await fetch(`https://wsl571-assignment3node-418610.wl.r.appspot.com/watch-company?company_id=${symbol}`);
                if (!response.ok) {
                    const errorData = await response.json();
                    throw new Error(errorData.message || `Error fetching data for ${symbol}: ${response.statusText}`);
                }
                const quoteResponse = await response.json();

                setCompanyQuote({
                    LastPrice: quoteResponse.c,
                    Change: quoteResponse.d,
                    ChangePercentage: quoteResponse.dp,
                    CurrentTimestamp: quoteResponse.t,
                    HighPrice: quoteResponse.h,
                    LowPrice: quoteResponse.l,
                    OpenPrice: quoteResponse.o,
                    PrevClose: quoteResponse.pc,
                    TimeStamp: quoteResponse.t
                });
            } catch (error) {
                console.error('Error fetching company data:', error);
            }
        };
        let interval;
        if (marketStatusMessage.includes('Open')) {
            fetchUpdatedData(Ticker); // Fetch immediately once if market is open
            interval = setInterval(() => fetchUpdatedData(Ticker), 15000); // Then set interval
        }

        return () => {
            if (interval) {
                clearInterval(interval);
            }
        };
        // Add Ticker and marketStatusMessage as dependencies to useEffect to ensure it updates accordingly
    }, [Ticker, marketStatusMessage]);
    const { Change, HighPrice, LowPrice, OpenPrice, PrevClose } = companyQuote;

    const chartColor = Change >= 0 ? 'green' : 'red';

    const formatPrice = (price) => isNaN(price) ? 'N/A' : Number(price).toFixed(2);

    const chartOptions = {
        chart: {
            type: 'line',
            backgroundColor: '#f0f0f0',
            scrollablePlotArea: {
                minWidth: 500,
                scrollPositionX: 1
            }

        },
        title: {
            text: `${Ticker} Hourly Price Variation`,
            style: {
                color: '#878787'
            }
        },
        xAxis: {
            type: 'datetime',
            dateTimeLabelFormats: {
                hour: '%H:%M',                                              //from ChatGPT
            },
            title: {
                text: ''
            },
            scrollbar: {
                enabled: true,
            },
            labels: {
                overflow: 'justify'
            }
        },
        yAxis: {
            title: {
                text: ''
            },
            opposite: true,
        },
        series: [{
            name: 'Stock Price',
            data: chartSeriesData,
            color: chartColor,
        }],
        plotOptions: {
            line: {
                dataLabels: {
                    enabled: false
                },
                marker: {
                    enabled: false
                },
                enableMouseTracking: false
            }
        },
        legend: {
            enabled: false
        },

    };

    return (
        <Container className="detail-container">
            <Row className="content-container">
                <Col xs={12} md={6} className="detail-content">
                    <div className="detail-price" style={{ width: '100%' }}>
                        <div><span style={{ fontWeight: 'bold' }}>High Price:</span> {formatPrice(HighPrice)}</div>
                        <div><span style={{ fontWeight: 'bold' }}>Low Price:</span> {formatPrice(LowPrice)}</div>
                        <div><span style={{ fontWeight: 'bold' }}>Open Price:</span> {formatPrice(OpenPrice)}</div>
                        <div><span style={{ fontWeight: 'bold' }}>Prev. Close:</span> {formatPrice(PrevClose)}</div>
                    </div>
                    <div className="detail-info" style={{ width: '100%' }}>
                        <p className="infoTitle">About the company</p>
                        <div><span style={{ fontWeight: 'bold' }}>IPO Start Date: </span>{IPOStartDate}</div>
                        <div><span style={{ fontWeight: 'bold' }}>Industry: </span>{Industry}</div>
                        <div><span style={{ fontWeight: 'bold' }}>Webpage: </span><a href={Webpage} target="_blank" rel="noopener noreferrer">{Webpage}</a></div>
                        <div><span style={{ fontWeight: 'bold' }}>Company Peers: </span>
                            <ul className="company-peers-list">
                                {companyPeer
                                    .filter(peer => !peer.includes("."))
                                    .map((peer, index) => (
                                        <li key={index} onClick={() => handleListItemClick(peer)} style={{ cursor: 'pointer' }}>
                                            {peer}
                                        </li>
                                    ))
                                }
                            </ul>
                        </div>
                    </div>
                </Col>
                <Col xs={12} md={6} className="chart-container">
                    <HighchartsReact highcharts={Highcharts} options={chartOptions} />
                </Col>
            </Row>
        </Container>
    );
};

export default DetailInfo;

