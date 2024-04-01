import moment from 'moment';

export const getMarketStatusMessage = (lastOpened) => {
    const baseDate = lastOpened ? moment(lastOpened) : moment();
    const now = moment();
    const today630am = baseDate.clone().startOf('day').add(6, 'hours').add(30, 'minutes');
    const today1pm = baseDate.clone().startOf('day').add(13, 'hours');

    if (now.isBetween(today630am, today1pm)) {
        return "Market is Open";
    } else if (now.isAfter(today1pm) && now.isBefore(baseDate.clone().endOf('day'))) {
        return `Market Closed on ${baseDate.format('YYYY-MM-DD 13:00:00')}`;
    } else {
        const yesterdayClose = now.isBefore(today630am) ? baseDate.clone().subtract(1, 'day').format('YYYY-MM-DD 13:00:00') : baseDate.format('YYYY-MM-DD 13:00:00');
        return `Market Closed on ${yesterdayClose}`;
    }
};

